/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVCODEC_V4L2_REQUEST_H
#define AVCODEC_V4L2_REQUEST_H

#include <stdatomic.h>
#include <stdbool.h>

#include <linux/videodev2.h>

#include "libavutil/hwcontext_drm.h"
#include "libavutil/thread.h"

// Below defines are custom patch from Collabora, as these are not included in the current AOSP, So better just
// here rather than messing with the headers in the AOSP build system. by - Venkata Atchuta Bheemeswara Sarma Darbha
#define V4L2_CID_STATELESS_HEVC_EXT_SPS_ST_RPS  (V4L2_CID_CODEC_STATELESS_BASE + 408)
#define V4L2_CID_STATELESS_HEVC_EXT_SPS_LT_RPS  (V4L2_CID_CODEC_STATELESS_BASE + 409)

#define V4L2_HEVC_EXT_SPS_ST_RPS_FLAG_INTER_REF_PIC_SET_PRED	0x1

#define V4L2_HEVC_EXT_SPS_LT_RPS_FLAG_USED_LT		0x1

#define	V4L2_CTRL_TYPE_HEVC_EXT_SPS_ST_RPS	= 0x0275,
#define	V4L2_CTRL_TYPE_HEVC_EXT_SPS_LT_RPS	= 0x0276,



struct v4l2_ctrl_hevc_ext_sps_st_rps {
	__u8	delta_idx_minus1;
	__u8	delta_rps_sign;
	__u16	abs_delta_rps_minus1;
	__u8	num_negative_pics;
	__u8	num_positive_pics;
	__u32	used_by_curr_pic;
	__u32	use_delta_flag;
	__u16	delta_poc_s0_minus1[16];
	__u16	delta_poc_s1_minus1[16];
	__u8	flags;
};

/*
 * struct v4l2_ctrl_hevc_ext_sps_lt_rps - HEVC long term RPS parameters
 *
 * Dynamic size 1-dimension array for long term RPS. The number of elements
 * is v4l2_ctrl_hevc_sps::num_long_term_ref_pics_sps. It can contain up to 65 elements.
 *
 * @lt_ref_pic_poc_lsb_sps: picture order count modulo MaxPicOrderCntLsb of the i-th candidate
 *                          long-term reference picture.
 * @flags: See V4L2_HEVC_EXT_SPS_LT_RPS_FLAG_{}
 */
struct v4l2_ctrl_hevc_ext_sps_lt_rps {
	__u16	lt_ref_pic_poc_lsb_sps;
	__u8	flags;
};


typedef struct V4L2RequestBuffer {
    int index;
    int fd;
    uint8_t *addr;
    uint32_t width;
    uint32_t height;
    uint32_t size;
    uint32_t used;
    uint32_t capabilities;
    struct v4l2_buffer buffer;
} V4L2RequestBuffer;

typedef struct V4L2RequestContext {
    const AVClass *av_class;
    AVBufferRef *device_ref;
    int media_fd;
    int video_fd;
    struct v4l2_format format;
    enum v4l2_buf_type output_type;
    AVMutex mutex;
    V4L2RequestBuffer output[4];
    atomic_int_least8_t next_output;
    atomic_uint_least32_t queued_output;
    atomic_uint_least32_t queued_request;
    atomic_uint_least64_t queued_capture;
    int (*post_probe)(AVCodecContext *avctx);
} V4L2RequestContext;

typedef struct V4L2RequestPictureContext {
    V4L2RequestBuffer *output;
    V4L2RequestBuffer *capture;
} V4L2RequestPictureContext;

int ff_v4l2_request_query_control(AVCodecContext *avctx,
                                  struct v4l2_query_ext_ctrl *control);

int ff_v4l2_request_query_control_default_value(AVCodecContext *avctx,
                                                uint32_t id);

int ff_v4l2_request_set_request_controls(V4L2RequestContext *ctx, int request_fd,
                                         struct v4l2_ext_control *control, int count);

int ff_v4l2_request_set_controls(AVCodecContext *avctx,
                                 struct v4l2_ext_control *control, int count);

int ff_v4l2_request_frame_params(AVCodecContext *avctx,
                                 AVBufferRef *hw_frames_ctx);

int ff_v4l2_request_uninit(AVCodecContext *avctx);

int ff_v4l2_request_init(AVCodecContext *avctx,
                         uint32_t pixelformat, uint32_t buffersize,
                         struct v4l2_ext_control *control, int count);

uint64_t ff_v4l2_request_get_capture_timestamp(AVFrame *frame);

int ff_v4l2_request_append_output(AVCodecContext *avctx,
                                  V4L2RequestPictureContext *pic,
                                  const uint8_t *data, uint32_t size);

int ff_v4l2_request_decode_slice(AVCodecContext *avctx,
                                 V4L2RequestPictureContext *pic,
                                 struct v4l2_ext_control *control, int count,
                                 bool first_slice, bool last_slice);

int ff_v4l2_request_decode_frame(AVCodecContext *avctx,
                                 V4L2RequestPictureContext *pic,
                                 struct v4l2_ext_control *control, int count);

int ff_v4l2_request_reset_picture(AVCodecContext *avctx,
                                  V4L2RequestPictureContext *pic);

int ff_v4l2_request_start_frame(AVCodecContext *avctx,
                                V4L2RequestPictureContext *pic, AVFrame *frame);

void ff_v4l2_request_flush(AVCodecContext *avctx);

#endif /* AVCODEC_V4L2_REQUEST_H */
