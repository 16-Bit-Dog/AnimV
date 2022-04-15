# AnimV
This is a project with the goal of analysing an image mathimatically for allowing free modifications of video content
... its not the best - nor worst working project out there... 
 
To Use Steps:
1. click `Open MP4` and choose .mp4 file
2. click `Open FFMPEG` to choose ffmpeg.exe
3. choose `Directory For Video Frame Output` to set directory of all output
4. click `Split Video Into Seperate Frames` <-- done for every new video loaded to split video into seperate frames
5. Click `Set Paths To Cache` to store all of the open'ed files for later use
6. check true `Save Video Images`
7. `Cache Video Images` takes lots of RAM but greatly speeds up code
8. choose Compute output to show --> num 0 and 1 are pixel frequency and rate of change
9. block_size is generally a bad idea to play around with
10. Sample count is amount of data forward and back to sample from (slower but accurate)
11. check num 0 and num 1 --> Compute accurate is slow but good for data accuracy of output 
12.  `run compute pass`
13.  choose FPS's of current and post --> use `soft body` frame interpolation for accurate attempt at interpolating

glfw license (All rights and material to glfw3 are reserved by the glfw orginization):
/*************************************************************************
 * GLFW 3.3 - www.glfw.org
 * A library for OpenGL, window and input
 *------------------------------------------------------------------------
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *************************************************************************/


imgui license (all rights are reserved to Omar Cornut):

The MIT License (MIT)

Copyright (c) 2014-2022 Omar Cornut

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.



ScreenGrab11 and the WICTextureLoader both are licensed under:
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
