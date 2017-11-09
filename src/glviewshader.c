/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <app.h>
#include <efl_extension.h>
#include "glviewshader.h"

/*
 * The file Elementary_GL_Helpers.h provies some convenience functions
 * that ease the use of OpenGL within Elementary application.
 */
#include <Elementary_GL_Helpers.h>

/*
 * ELEMENTARY_GLVIEW_GLOBAL_DEFINE() is
 * #define ELEMENTARY_GLVIEW_GLOBAL_DEFINE() \
 *  Evas_GL_API *__evas_gl_glapi = NULL;
 */
ELEMENTARY_GLVIEW_GLOBAL_DEFINE();

const float EPSILON = 0.5f;
const int PAUSE_TIME = 100;

/* Vertex Shader Source */
static const char vertex_shader[] =
	 "attribute vec4 vPosition; \n"
	 "void main() \n"
	 "{ \n"
	 " gl_Position = vPosition; \n"
	 "} \n";

/* Fragment Shader Source */
static const char fragment_shader[] =
	"precision mediump float; \n"
	 "void main() \n"
	 "{ \n"
	 " gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); \n"
	 "} \n";

/*
 * @brief Hide the window when back button is pressed
 * @param[in] data App data
 * @param[in] obj Elm Window object
 * @param[in] event_info Not use
 */
static void win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hidden state. */
	elm_win_lower(ad->win);
}

/*
 * @brief Initialize vertex & fragment shaders
 * @param[in] obj GLView object
 */
static void init_shaders(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");
	const char *p;

	/* Create & compile vertex shader */
	p = vertex_shader;
	ad->vtx_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(ad->vtx_shader, 1, &p, NULL);
	glCompileShader(ad->vtx_shader);

	/* Create & compile fragment shader */
	p = fragment_shader;
	ad->fgmt_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(ad->fgmt_shader, 1, &p, NULL);
	glCompileShader(ad->fgmt_shader);

	/* Create a program and
	 * attach vertex and fragment shaders to the program
	 */
	ad->program = glCreateProgram();
	glAttachShader(ad->program, ad->vtx_shader);
	glAttachShader(ad->program, ad->fgmt_shader);

	glLinkProgram(ad->program);

	/* Use the program */
	glUseProgram(ad->program);
}

/*
 * @brief Callback function to be invoked when size of glview is resized
 * @param[in] obj GLView object
 */
static void resize_gl(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	/* Get size of GLView object for setting Viewport*/
	elm_glview_size_get(obj, &ad->glview_w, &ad->glview_h);
}

/*
 * @brief Drawing function of GLView
 * @param[in] obj GLView object
 */
static void draw_gl(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	GLfloat vVertices[] = {0.0f, 0.5f, 0.0f,
	 -0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f};

	glViewport(0, 0, ad->glview_w, ad->glview_h);
	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);
	// Use the program object
	glUseProgram(ad->program);
	// Load the vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

/*
 * @brief Initializing function of GLView
 * @param[in] obj GLView object
 */
static void init_gl(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	if (!ad->initialized) {
		init_shaders(obj);
		ad->initialized = EINA_TRUE;
	}
}

/*
 * @brief Callback function to be invoked when glview object is deleted
 * @param[in] obj GLView object
 */
static void del_gl(Evas_Object *obj)
{
	appdata_s *ad = evas_object_data_get(obj, "ad");

	/* Release resources. */
	glDeleteShader(ad->vtx_shader);
	glDeleteShader(ad->fgmt_shader);
	glDeleteProgram(ad->program);

	evas_object_data_del((Evas_Object*) obj, "ad");
}

static void
win_delete_request_cb(void *data , Evas_Object *obj , void *event_info)
{
	ui_app_exit();
}

/*
 * @brief Create a indicator
 * param[in] ad app data
 */
static void create_indicator(appdata_s *ad)
{
	elm_win_conformant_set(ad->win, EINA_TRUE);

	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_TRANSPARENT);

	ad->conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);
}

/*
 * @brief Create a window
 * param[in] name Window name
 */
static Evas_Object* add_win(const char *name)
{
	Evas_Object *win;

	/*
	 * To use the Direct Rendering mode of GLView,
	 * set the same option values (depth, stencil, and MSAA)
     * to a rendering engine and a GLView object.
	 */
	elm_config_accel_preference_set("opengl:depth");
	win = elm_win_util_standard_add(name, "OpenGL example: Tea pot");

	if (!win)
		return NULL;

	if (elm_win_wm_rotation_supported_get(win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(win, rots, 4);
	}

	evas_object_show(win);

	return win;
}

static bool app_create(void *data)
{
	/*
	 * Hook to take necessary actions before main event loop starts
	 * Initialize UI resources and application's data
	 * If this function returns true, the main loop of application starts
	 * If this function returns false, the application is terminated.
	 */

	Evas_Object *gl;
	appdata_s *ad = data;

	if (!data)
		return false;

	/* Create the window */
	ad->win = add_win(ad->name);

	if (!ad->win)
		return false;

	create_indicator(ad);
	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Create and initialize GLView */
	gl = elm_glview_add(ad->conform);

	/*
	 * ELEMENTARY_GLVIEW_GLOBAL_USE() is
	 * #define ELEMENTARY_GLVIEW_USE(glview) \
	 *  Evas_GL_API *__evas_gl_glapi = elm_glview_gl_api_get(glview);
	 */
	ELEMENTARY_GLVIEW_GLOBAL_USE(gl);
	evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/*
	 * Request a surface with a depth buffer
	 *
	 * To use the Direct Rendering mode, set the same option values (depth, stencil, and MSAA)
	 * to a rendering engine and a GLView object.
	 * You can set the option values to a rendering engine
	 * using the elm_config_accel_preference_set() function and
	 * to a GLView object using the elm_glview_mode_set() function.
	 * If the GLView object option values are bigger or higher than the rendering engine's,
	 * the Direct Rendering mode is disabled.
	 */
	elm_glview_mode_set(gl, ELM_GLVIEW_DEPTH | ELM_GLVIEW_DIRECT | ELM_GLVIEW_CLIENT_SIDE_ROTATION);

	/*
	 * The resize policy tells GLView what to do with the surface when it
	 * resizes. ELM_GLVIEW_RESIZE_POLICY_RECREATE will tell it to
	 * destroy the current surface and recreate it to the new size.
	 */
	elm_glview_resize_policy_set(gl, ELM_GLVIEW_RESIZE_POLICY_RECREATE);

	/*
	 * The render policy sets how GLView should render GL code.
	 * ELM_GLVIEW_RENDER_POLICY_ON_DEMAND will have the GL callback
	 * called only when the object is visible.
	 * ELM_GLVIEW_RENDER_POLICY_ALWAYS would cause the callback to be
	 * called even if the object were hidden.
	 */
	elm_glview_render_policy_set(gl, ELM_GLVIEW_RENDER_POLICY_ON_DEMAND);

	/* The initialize callback function gets registered here */
	elm_glview_init_func_set(gl, init_gl);

	/* The delete callback function gets registered here */
	elm_glview_del_func_set(gl, del_gl);

	/* The resize callback function gets registered here */
	elm_glview_resize_func_set(gl, resize_gl);

	/* The render callback function gets registered here */
	elm_glview_render_func_set(gl, draw_gl);

	/* Add the GLView to the conformant and show it */
	elm_object_content_set(ad->conform, gl);
	evas_object_show(gl);

	elm_object_focus_set(gl, EINA_TRUE);

	/* This adds an animator so that the app will regularly
	 * trigger updates of the GLView using elm_glview_changed_set().
	 *
	 * NOTE: If you delete GL, this animator will keep running trying to access
	 * GL so this animator needs to be deleted with ecore_animator_del().
	 */

	//ad->ani = ecore_animator_add(anim, gl);
	evas_object_data_set(gl, "ad", ad);
	//evas_object_event_callback_add(gl, EVAS_CALLBACK_DEL, del_anim, ad);

	evas_object_show(ad->win);

	/* Return true: the main loop will now start running */
	return true;
}

static void app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void app_pause(void *data)
{
	appdata_s *ad = data;

	/*
	 * When app is paused,
	 * Freeze animator for power saving
	 */
	ecore_animator_freeze(ad->ani);
}

static void app_resume(void *data)
{
	appdata_s *ad = data;
	/* When app is resumed, thaw animator */
	ecore_animator_thaw(ad->ani);
}

static void app_terminate(void *data)
{
	/* Release all resources. */
}

int main(int argc, char *argv[])
{
	int ret = 0;
	appdata_s ad = { NULL, };
	ui_app_lifecycle_callback_s event_callback = {NULL,};

	ad.name = "glviewshader";

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "The application failed to start, and returned %d", ret);

	return ret;
}
