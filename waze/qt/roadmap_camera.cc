/* roadmap_camera.c - Android camera API implementation
 *
 * LICENSE:
 *   Copyright 2009, Waze Ltd
 *   Alex Agranovich
 *   Ehud Shabtai
 *
 *   This file is part of RoadMap.
 *
 *   RoadMap is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License V2 as published by
 *   the Free Software Foundation.
 *
 *   RoadMap is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with RoadMap; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * SYNOPSYS:
 *
 *   See roadmap_camera.h
 */

#include <stdlib.h>
#include <string.h>
extern "C" {
#include "roadmap_camera.h"
#include "roadmap_path.h"
#include "roadmap_camera_image.h"
#include "roadmap_path.h"
}

// Temporary file for the image capture
static const char* gCaptureFileName = "capture_temp.jpg";

/*
 * JNI Callback context
 */
typedef struct
{
   CameraImageCaptureCallback capture_callback;
   CameraImageCaptureContext* context;
} AndrCameraCbContext;

/*
 * Only one camera capture can be done at a time. So can be assume singelton
 */
static AndrCameraCbContext sgCameraContext;

/***********************************************************
 *  Name        : roadmap_camera_take_picture
 *  Purpose     : Shows the camera preview, passes target image attributes
 *                   and defines the target capture file location
 *
 */
BOOL roadmap_camera_take_picture( CameraImageFile* image_file, CameraImageBuf* image_thumbnail )
{
    /* TODO */
    return FALSE;
}



/***********************************************************
 *  Name        : roadmap_camera_take_picture_async
 *  Purpose     : Asynchronous function. Main thread still running.
 *                Shows the camera preview, passes target image attributes
 *                   and defines the target capture file location
 *
 */
BOOL roadmap_camera_take_picture_async( CameraImageCaptureCallback callback, CameraImageCaptureContext* context )
{
    /* TODO */

    return TRUE;
}


void roadmap_camera_take_picture_async_cb( int res )
{
   /* TODO */
}

