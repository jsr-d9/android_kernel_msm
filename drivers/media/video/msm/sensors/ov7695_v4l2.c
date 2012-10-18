/* Copyright (c) 2012, The Linux Foundation. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "msm_sensor.h"
#define SENSOR_NAME "ov7695"

DEFINE_MUTEX(ov7695_mut);
static struct msm_sensor_ctrl_t ov7695_s_ctrl;

static struct msm_camera_i2c_reg_conf ov7695_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf ov7695_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf ov7695_recommend_settings[] = {
	{0x0103, 0x01}, 	//	software reset
	{0x3620, 0x2f},
	{0x3623, 0x12},
	{0x3718, 0x88},
	{0x3631, 0x44},
	{0x3632, 0x05},
	{0x3013, 0xd0},
	{0x3705, 0x1d},
	{0x3713, 0x0e},
	{0x3012, 0x0a},
	{0x3717, 0x19},
	{0x0309, 0x24}, 	//	DAC clk div by 4
	{0x3820, 0x90},
	{0x0101, 0x01}, 	//	mirror on, flip off
	{0x5100, 0x01}, 	//	lenc
	{0x520a, 0xf4}, 	//	red gain from 0x400 to 0xfff
	{0x520b, 0xf4}, 	//	green gain from 0x400 to 0xfff
	{0x520c, 0xf4}, 	//	blue gain from 0x400 to 0xfff
	{0x3a18, 0x01}, 	//	gain ceiling 0x100
	{0x3a19, 0x00}, 	//	gain ceiling
	{0x3503, 0x03}, 	//	AGC manual on, AEC manual on
	{0x3500, 0x00}, 	//	exposure
	{0x3501, 0x21}, 	//	exposure
	{0x3502, 0x00}, 	//	exposure
	{0x350a, 0x00}, 	//	gain
	{0x350b, 0x00}, 	//	gain
	{0x4008, 0x02}, 	//	bl start
	{0x4009, 0x09}, 	//	bl end
	{0x3002, 0x09}, 	//	FSIN output
	{0x3024, 0x00},
	{0x3503, 0x00}, 	//	AGC auto on, AEC auto on
	// OV7695_ISP
	{0x0101, 0x01}, 	//	mirror_on
	{0x5002, 0x40}, 	//	[7:6] Y source select, manual 60Hz
	{0x5910, 0x00}, 	//	Y formula
	{0x3a0f, 0x58}, 	//	AEC in H
	{0x3a10, 0x50}, 	//38 ;AEC in L
	{0x3a1b, 0x5a}, 	//40 ;AEC out H
	{0x3a1e, 0x4e}, 	//36 ;AEC out L
	{0x3a11, 0xa0}, 	//80 ;control zone H
	{0x3a1f, 0x28}, 	//18 ;control zone L
	{0x3a18, 0x00}, 	//	gain ceiling
	{0x3a19, 0xf8}, 	//	gain ceiling, max gain 15.5x
	{0x3503, 0x00}, 	//	aec/agc auto on
	{0x5000, 0xff}, 	//	lcd, gma, awb, awbg, bc, wc, lenc, isp
	{0x5001, 0x3f}, 	//	avg, blc, sde, uv_avg, cmx, cip
	//lens
	{0x5100, 0x01},
	{0x5101, 0xbf},
	{0x5102, 0x00},
	{0x5103, 0xaa},
	{0x5104, 0x3f},
	{0x5105, 0x05},
	{0x5106, 0xff},
	{0x5107, 0x0f},
	{0x5108, 0x01},
	{0x5109, 0xff},
	{0x510a, 0x00},
	{0x510b, 0x72},
	{0x510c, 0x45},
	{0x510d, 0x06},
	{0x510e, 0xff},
	{0x510f, 0x0f},
	{0x5110, 0x01},
	{0x5111, 0xfe},
	{0x5112, 0x00},
	{0x5113, 0x70},
	{0x5114, 0x21},
	{0x5115, 0x05},
	{0x5116, 0xff},
	{0x5117, 0x0f},
	//AWB
	{0x520a, 0x74}, 	//	red gain from 0x400 to 0x7ff
	{0x520b, 0x64}, 	//	green gain from 0x400 to 0x7ff
	{0x520c, 0xd4}, 	//	blue gain from 0x400 to 0xdff
	//Gamma
	{0x5301, 0x05},
	{0x5302, 0x0c},
	{0x5303, 0x1c},
	{0x5304, 0x2a},
	{0x5305, 0x39},
	{0x5306, 0x45},
	{0x5307, 0x53},
	{0x5308, 0x5d},
	{0x5309, 0x68},
	{0x530a, 0x7f},
	{0x530b, 0x91},
	{0x530c, 0xa5},
	{0x530d, 0xc6},
	{0x530e, 0xde},
	{0x530f, 0xef},
	{0x5310, 0x16},
	//sharpen/denoise
	{0x5500, 0x08}, 	//	sharp th1 8x
	{0x5501, 0x48}, 	//	sharp th2 8x
	{0x5502, 0x18}, 	//	sharp mt offset1
	{0x5503, 0x04}, 	//	sharp mt offset2
	{0x5504, 0x08}, 	//	dns th1 8x
	{0x5505, 0x48}, 	//	dns th2 8x
	{0x5506, 0x02}, 	//	dns offset1
	{0x5507, 0x16}, 	//	dns offset2
	{0x5508, 0xad}, 	//	[6]:sharp_man [4]:dns_man
	{0x5509, 0x08}, 	//	sharpth th1 8x
	{0x550a, 0x48}, 	//	sharpth th2 8x
	{0x550b, 0x06}, 	//	sharpth offset1
	{0x550c, 0x04}, 	//	sharpth offset2
	{0x550d, 0x01}, 	//	recursive_en
	//SDE, for saturation 120% under D65
	{0x5800, 0x06}, 	//	saturation on, contrast on
	{0x5803, 0x2e}, 	//	40 ; sat th2
	{0x5804, 0x20}, 	//	34 ; sat th1
	{0x580b, 0x02}, 	//	Y offset man on
	// CMX QE
	{0x5600, 0x00}, 	//	mtx 1.7, UV CbCr disable
	{0x5601, 0x2c}, 	//	CMX1
	{0x5602, 0x5a}, 	//	CMX2
	{0x5603, 0x06}, 	//	CMX3
	{0x5604, 0x1c}, 	//	CMX4
	{0x5605, 0x65}, 	//	CMX5
	{0x5606, 0x81}, 	//	CMX6
	{0x5607, 0x9f}, 	//	CMX7
	{0x5608, 0x8a}, 	//	CMX8
	{0x5609, 0x15}, 	//	CMX9
	{0x560a, 0x01}, 	//	Sign
	{0x560b, 0x9c}, 	//	Sign
	{0x3811, 0x07}, 	//	Tradeoff position to make YUV/RAW x VGA/QVGA x Mirror/Flip all work
	{0x3813, 0x06},
	{0x3a05, 0xb0}, 	//	banding filter 50hz
	//	MIPI
	{0x4800, 0x20},
	{0x4801, 0x0e},
	{0x4802, 0x14},
	{0x4803, 0x0a},
	{0x4804, 0x0a},
	{0x4805, 0x0a},
	{0x4806, 0x30},
	{0x4807, 0x05},
	{0x0100, 0x01}, 	//	streaming
};

static struct msm_camera_i2c_reg_conf ov7695_full_settings[] = {
	{0x034c, 0x02},
	{0x034d, 0x80},
	{0x034e, 0x01},
	{0x034f, 0xe0},
	{0x0340, 0x02},
	{0x0341, 0x18},
	{0x0342, 0x02},
	{0x0343, 0xea},
};


static struct v4l2_subdev_info ov7695_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
	/* more can be supported, to be added later */
};


static struct msm_camera_i2c_conf_array ov7695_init_conf[] = {
	{&ov7695_recommend_settings[0],
	ARRAY_SIZE(ov7695_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};


static struct msm_camera_i2c_conf_array ov7695_confs[] = {
	{&ov7695_full_settings[0],
	ARRAY_SIZE(ov7695_full_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t ov7695_dimensions[] = {
	{
		.x_output = 0x280,
		.y_output = 0x1E0,
		.line_length_pclk = 0x2EA,
		.frame_length_lines = 0x218,
		.vt_pixel_clk = 12000000,
		.op_pixel_clk = 9216000,
		.binning_factor = 0,
	},
};

static struct msm_camera_i2c_reg_conf ov7695_no_effect[] = {

};

static struct msm_camera_i2c_conf_array ov7695_no_effect_confs[] = {
	{&ov7695_no_effect[0],
	ARRAY_SIZE(ov7695_no_effect), 0,
	MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA},
};

static struct msm_camera_csi_params ov7695_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 0x19,
};

static struct msm_camera_csi_params *ov7695_csi_params_array[] = {
	&ov7695_csi_params,
};

static struct msm_sensor_output_reg_addr_t ov7695_reg_addr = {
	.x_output = 0x034C,
	.y_output = 0x034E,
	.line_length_pclk = 0x0342,
	.frame_length_lines = 0x0340,
};

static struct msm_sensor_id_info_t ov7695_id_info = {
	.sensor_id_reg_addr = 0x300A,
	.sensor_id = 0x7695,
};

static const struct i2c_device_id ov7695_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov7695_s_ctrl},
	{ }
};


static struct i2c_driver ov7695_i2c_driver = {
	.id_table = ov7695_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov7695_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	int rc = 0;
	CDBG("ov7695\n");

	rc = i2c_add_driver(&ov7695_i2c_driver);

	return rc;
}

static struct v4l2_subdev_core_ops ov7695_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov7695_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov7695_subdev_ops = {
	.core = &ov7695_subdev_core_ops,
	.video  = &ov7695_subdev_video_ops,
};

static struct msm_sensor_fn_t ov7695_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_csi_setting = msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t ov7695_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov7695_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov7695_start_settings),
	.stop_stream_conf = ov7695_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov7695_stop_settings),
	.init_settings = &ov7695_init_conf[0],
	.init_size = ARRAY_SIZE(ov7695_init_conf),
	.mode_settings = &ov7695_confs[0],
	.no_effect_settings = &ov7695_no_effect_confs[0],
	.output_settings = &ov7695_dimensions[0],
	.num_conf = ARRAY_SIZE(ov7695_confs),
};

static struct msm_sensor_ctrl_t ov7695_s_ctrl = {
	.msm_sensor_reg = &ov7695_regs,
	.sensor_i2c_client = &ov7695_sensor_i2c_client,
	.sensor_i2c_addr = 0x42,
	.sensor_output_reg_addr = &ov7695_reg_addr,
	.sensor_id_info = &ov7695_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &ov7695_csi_params_array[0],
	.msm_sensor_mutex = &ov7695_mut,
	.sensor_i2c_driver = &ov7695_i2c_driver,
	.sensor_v4l2_subdev_info = ov7695_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov7695_subdev_info),
	.sensor_v4l2_subdev_ops = &ov7695_subdev_ops,
	.func_tbl = &ov7695_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Omnivision VGA YUV sensor driver");
MODULE_LICENSE("GPL v2");
