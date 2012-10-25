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
#include "msm.h"
#define SENSOR_NAME "ov7695_raw"

#ifdef CDBG
#undef CDBG
#endif
#ifdef CDBG_HIGH
#undef CDBG_HIGH
#endif

//#define ov7695_raw_VERBOSE_DGB

#ifdef ov7695_raw_VERBOSE_DGB
#define CDBG(fmt, args...) printk(fmt, ##args)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#endif

static struct msm_sensor_ctrl_t ov7695_raw_s_ctrl;
DEFINE_MUTEX(ov7695_raw_mut);
static struct msm_camera_i2c_reg_conf ov7695_raw_start_settings[] = {
	{0x0100, 0x01},
};

static struct msm_camera_i2c_reg_conf ov7695_raw_stop_settings[] = {
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf ov7695_raw_recommend_settings[] = {
	{0x0103, 0x01}, 	//software reset
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
	{0x4300, 0xf8}, 	//RGB raw output
	{0x030b, 0x04}, 	//pre_sys_clk div
	{0x3106, 0x91}, 	//yuv_clk_select
	{0x301e, 0x60},
	{0x0309, 0x24}, 	//dac_clk_div
	{0x3820, 0x90}, 	//binning off
	{0x4803, 0x08}, 	//MIPI HS Prepare
	{0x0101, 0x01}, 	//Mirror on, flip off
	{0x5000, 0x1f}, 	//lcdc off, gamma off, awb off, awb gain on
	{0x5200, 0x20}, 	//awb gain manual on
	{0x5204, 0x04}, 	//red gain = 0x400
	{0x5205, 0x00}, 	//red gain
	{0x5206, 0x04}, 	//green gain = 0x400
	{0x5207, 0x00}, 	//green gain
	{0x5208, 0x04}, 	//blue gain = 0x400
	{0x5209, 0x00}, 	//blue gain
	{0x3a18, 0x01}, 	//gain ceiling = 32x
	{0x3a19, 0x00}, 	//gain ceiling
	{0x3503, 0x03}, 	//AGC/AEC manual on
	{0x3500, 0x00}, 	//exposure
	{0x3501, 0x21}, 	//exposure
	{0x3502, 0x00}, 	//exposure
	{0x350a, 0x00}, 	//gain
	{0x350b, 0x00}, 	//gain
	{0x4008, 0x02}, 	//bl start
	{0x4009, 0x09}, 	//bl end
	{0x3002, 0x09}, 	//FSIN output
	{0x3024, 0x00},
	{0x3503, 0x00}, 	//AGC/AEC auto on
	{0x3811, 0x07}, 	//Tradeoff position to make YUV/RAW x VGA/QVGA x Mirror/Flip all work
	{0x3813, 0x06},
	{0x3a05, 0xb0}, 	//banding filter 50hz
	{0x4800, 0x20},		//MIPI
	{0x4801, 0x0e},
	{0x4802, 0x14},
	{0x4803, 0x0a},
	{0x4804, 0x0a},
	{0x4805, 0x0a},
	{0x4806, 0x30},
	{0x4807, 0x05},
	{0x0100, 0x01}, 	//MIPI stream on
};
static struct msm_camera_i2c_reg_conf ov7695_raw_full_settings[] = {
	{0x034c, 0x02},
	{0x034d, 0x80},
	{0x034e, 0x01},
	{0x034f, 0xe0},
	{0x0340, 0x02},
	{0x0341, 0x18},
	{0x0342, 0x02},
	{0x0343, 0xea},
};

static struct msm_camera_i2c_conf_array ov7695_raw_confs[] = {
	{&ov7695_raw_full_settings[0],
	ARRAY_SIZE(ov7695_raw_full_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_i2c_conf_array ov7695_raw_init_conf[] = {
	{&ov7695_raw_recommend_settings[0],
	ARRAY_SIZE(ov7695_raw_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_csi_params ov7695_raw_csi_params = {
	.data_format	=	CSI_8BIT,
	.lane_cnt		=	1,
	.lane_assign	=	0xe4,
	.dpcm_scheme	=	0,
	.settle_cnt		=	0x19,
};

static struct v4l2_subdev_info ov7695_raw_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static struct msm_sensor_output_info_t ov7695_raw_dimensions[] = {
	{
		.x_output                = 0x280,
		.y_output                = 0x1E0,
		.line_length_pclk        = 0x2EA,
		.frame_length_lines      = 0x218,
		.vt_pixel_clk            = 12000000,
		.op_pixel_clk            = 12000000,
		.binning_factor          = 0,
	},
};

static struct msm_sensor_output_reg_addr_t ov7695_raw_reg_addr = {
	.x_output			=	0x034C,
	.y_output			=	0x034E,
	.line_length_pclk	=	0x0342,
	.frame_length_lines	=	0x0340,
};

static struct msm_camera_csi_params *ov7695_raw_csi_params_array[] = {
	&ov7695_raw_csi_params,
};

static struct msm_sensor_id_info_t ov7695_raw_id_info = {
	.sensor_id_reg_addr	=	0x300A,
	.sensor_id			=	0x7695,
};

static struct msm_sensor_exp_gain_info_t ov7695_raw_exp_gain_info = {
	.coarse_int_time_addr      = 0x0000,
	.global_gain_addr          = 0x0000,
	.vert_offset               = 0,
};

static int32_t ov7695_raw_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t gain, uint32_t line)
{
	int rc = 0;
	//unsigned int  intg_time_msb, intg_time_lsb;
	CDBG("ov7695_raw_write_pict_exp_gain,gain=%d, line=%d\n",gain,line);
#if 0
	intg_time_msb = (unsigned int ) ((line & 0x0F00) >> 8);
	intg_time_lsb = (unsigned int ) (line& 0x00FF);
	if(gain>0xff)
		gain=0xff;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x51,(gain),MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x03,(intg_time_msb),MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x04,(intg_time_lsb),MSM_CAMERA_I2C_BYTE_DATA);
#endif
	return rc;
}

static int32_t ov7695_raw_write_prev_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t gain, uint32_t line)
{
	int rc = 0;
	//unsigned int  intg_time_msb, intg_time_lsb;
	CDBG("ov7695_raw_write_prev_exp_gain,gain=%d, line=%d\n",gain,line);
#if 0
	intg_time_msb = (unsigned int ) ((line & 0x0F00) >> 8);
	intg_time_lsb = (unsigned int ) (line& 0x00FF);
	if(gain>0xff)
		gain=0xff;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x51,(gain),MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x03,(intg_time_msb),MSM_CAMERA_I2C_BYTE_DATA);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x04,(intg_time_lsb),MSM_CAMERA_I2C_BYTE_DATA);
#endif
	return rc;
}

static const struct i2c_device_id ov7695_raw_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov7695_raw_s_ctrl},
	{ }
};

static struct i2c_driver ov7695_raw_i2c_driver = {
	.id_table = ov7695_raw_i2c_id,
	.probe = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov7695_raw_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&ov7695_raw_i2c_driver);
}

static struct v4l2_subdev_core_ops ov7695_raw_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov7695_raw_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov7695_raw_subdev_ops = {
	.core = &ov7695_raw_subdev_core_ops,
	.video  = &ov7695_raw_subdev_video_ops,
};

static struct msm_sensor_fn_t ov7695_raw_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = ov7695_raw_write_prev_exp_gain,
	.sensor_write_snapshot_exp_gain = ov7695_raw_write_pict_exp_gain,
	.sensor_csi_setting = msm_sensor_setting1,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_match_id   = msm_sensor_match_id,
};

static struct msm_sensor_reg_t ov7695_raw_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov7695_raw_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov7695_raw_start_settings),
	.stop_stream_conf = ov7695_raw_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov7695_raw_stop_settings),
	.init_settings = &ov7695_raw_init_conf[0],
	.init_size = ARRAY_SIZE(ov7695_raw_init_conf),
	.mode_settings = &ov7695_raw_confs[0],
	.output_settings = &ov7695_raw_dimensions[0],
	.num_conf = ARRAY_SIZE(ov7695_raw_confs),
};

static struct msm_sensor_ctrl_t ov7695_raw_s_ctrl = {
	.msm_sensor_reg = &ov7695_raw_regs,
	.sensor_i2c_client = &ov7695_raw_sensor_i2c_client,
	.sensor_i2c_addr =  0x42,
	.sensor_output_reg_addr = &ov7695_raw_reg_addr,
	.sensor_id_info = &ov7695_raw_id_info,
	.sensor_exp_gain_info = &ov7695_raw_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &ov7695_raw_csi_params_array[0],
	.msm_sensor_mutex = &ov7695_raw_mut,
	.sensor_i2c_driver = &ov7695_raw_i2c_driver,
	.sensor_v4l2_subdev_info = ov7695_raw_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov7695_raw_subdev_info),
	.sensor_v4l2_subdev_ops = &ov7695_raw_subdev_ops,
	.func_tbl = &ov7695_raw_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Omnivision BAYER sensor driver");
MODULE_LICENSE("GPL v2");
