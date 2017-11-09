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

#ifndef __glviewshader_H__
#define __glviewshader_H__

#include <dlog.h>
#include <Elementary.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "glviewshader"

typedef struct appdata {
	const char *name;

	Evas_Object *win;
	Evas_Object *conform;
	Ecore_Animator *ani;
	int glview_h, glview_w, view_rotation;

	/* GL related data here... */
	unsigned int program;
	unsigned int vtx_shader;
	unsigned int fgmt_shader;

	float angle;

	unsigned int idx_vbo;
	unsigned int idx_ibo;
	unsigned int idx_vposition;
	unsigned int idx_vnormal;
	int idx_light_dir;
	int idx_mvp;
	int idx_time_stamp;

	float mvp[16];

	float light_dir[4];

	int stop_count;

	float time_stamp;
	float stride_time_stamp;

	Eina_Bool initialized;
} appdata_s;

#endif /* __glviewshader_H__ */
