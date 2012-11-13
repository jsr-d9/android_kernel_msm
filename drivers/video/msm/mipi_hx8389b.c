/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

#define DEBUG

#include <mach/socinfo.h>
#include <linux/gpio.h>
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_hx8389b.h"

static struct msm_panel_common_pdata *mipi_hx8389b_pdata;
static struct dsi_buf hx8389b_tx_buf;
static struct dsi_buf hx8389b_rx_buf;

static int mipi_hx8389b_bl_ctrl = 0;

/* common setting */
static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

/* panel setting */
static char video0[] = {
        0xB9, 0xFF, 0x83, 0x89,
};

static char video1[] = {
        0xB1, 0x00, 0x00, 0x04,
        0xE8, 0x50, 0x10, 0x11,
        0xb0, 0xf0, 0x2b, 0x33,
        0x1a, 0x1a, 0x43, 0x01,
        0x58, 0xF2, 0x00, 0xE6,
};

static char video2[] = {
        0xB2, 0x00, 0x00, 0x78,
        0x0C, 0x07, 0x00, 0x30,
};

static char video3[] = {
        0xB4, 0x80, 0x08, 0x00,
        0x32, 0x10, 0x04, 0x32,
        0x10, 0x00, 0x32, 0x10,
        0x00, 0x37, 0x0A, 0x40,
        0x08, 0x37, 0x0A, 0x40,
        0x14, 0x46, 0x50, 0x0A,
};

static char video4[] = {
        0xD5, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x60, 0x00, 0x88,
        0x88, 0x88, 0x88, 0x88,
        0x23, 0x88, 0x01, 0x88,
        0x67, 0x88, 0x45, 0x01,
        0x23, 0x88, 0x88, 0x88,
        0x88, 0x88, 0x88, 0x88,
        0x88, 0x88, 0x88, 0x54,
        0x88, 0x76, 0x88, 0x10,
        0x88, 0x32, 0x32, 0x10,
        0x88, 0x88, 0x88, 0x88,
        0x88, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00,
};

static char video5[] = {0xCB, 0x07, 0x07};

static char video6[] = {0xBB, 0x00, 0x00, 0xFF, 0x80};

static char video7[] = {0xDE, 0x05, 0x58};

static char video8[] = {0xB6, 0x00, 0xA4, 0x00, 0xA4};

static char video9[] = {
        0xE0, 0x05, 0x07, 0x16,
        0x2d, 0x2b, 0x3f, 0x39,
        0x4c, 0x06, 0x12, 0x18,
        0x19, 0x1a, 0x17, 0x18,
        0x10, 0x16, 0x05, 0x07,
        0x16, 0x2d, 0x2b, 0x3f,
        0x39, 0x4c, 0x06, 0x12,
        0x18, 0x19, 0x1a, 0x17,
        0x18, 0x10, 0x16,
};

static char video10[] = {
        0xC1, 0x01, 0x03, 0x05,
        0x0D, 0x16, 0x1C, 0x27,
        0x31, 0x38, 0x43, 0x4B,
        0x56, 0x60, 0x6A, 0x74,
        0x7D, 0x87, 0x8F, 0x97,
        0x9E, 0xA8, 0xB1, 0xBA,
        0xC2, 0xCA, 0xD1, 0xD9,
        0xE0, 0xE4, 0xEA, 0xF1,
        0xF5, 0xFB, 0xFF, 0x00,
        0x15, 0x2A, 0xEC, 0x0D,
        0x49, 0x4A, 0x45, 0x00,
        0x00, 0x03, 0x0A, 0x12,
        0x19, 0x1F, 0x2A, 0x32,
        0x39, 0x42, 0x4A, 0x54,
        0x5D, 0x66, 0x6F, 0x78,
        0x80, 0x88, 0x8F, 0x96,
        0x9F, 0xA7, 0xB0, 0xB8,
        0xC1, 0xC9, 0xD1, 0xDA,
        0xE2, 0xE7, 0xEF, 0xF7,
        0xFD, 0x52, 0xDB, 0xA9,
        0x54, 0x57, 0x16, 0x64,
        0x56, 0x80, 0x01, 0x01,
        0x07, 0x0F, 0x17, 0x1B,
        0x26, 0x2C, 0x34, 0x3B,
        0x43, 0x4B, 0x54, 0x5D,
        0x66, 0x6F, 0x78, 0x80,
        0x88, 0x90, 0x96, 0x9A,
        0xA3, 0xAB, 0xB4, 0xBD,
        0xC6, 0xCE, 0xD7, 0xE1,
        0xE7, 0xF2, 0xFE, 0x00,
        0x23, 0x08, 0xBE, 0x7A,
        0xE7, 0xE2, 0x3E, 0x80,
};

static char video11[] = {0x21, 0x00};

/* HX8389B must use DCS commands */
static struct dsi_cmd_desc hx8389b_video_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video0), video0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video1), video1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video2), video2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video3), video3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video4), video4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video5), video5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video6), video6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video7), video7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video8), video8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video9), video9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video10), video10},
	{DTYPE_DCS_WRITE,  1, 0, 0, 0, sizeof(video11), video11},
	{DTYPE_DCS_WRITE,  1, 0, 0, 150, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE,  1, 0, 0, 150, sizeof(display_on), display_on},
};

static struct dsi_cmd_desc hx8389b_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};

static int mipi_hx8389b_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;

	pr_debug("mipi_hx8389b_lcd_on E\n");

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;

	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi  = &mfd->panel_info.mipi;

	if (!mfd->cont_splash_done) {
		mfd->cont_splash_done = 1;
		return 0;
	}

	if (mipi->mode == DSI_VIDEO_MODE) {
		mipi_dsi_cmds_tx(&hx8389b_tx_buf,
			hx8389b_video_display_on_cmds,
			ARRAY_SIZE(hx8389b_video_display_on_cmds));
	}

	pr_debug("mipi_hx8389b_lcd_on X\n");

	return 0;
}

static int mipi_hx8389b_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	pr_debug("mipi_hx8389b_lcd_off E\n");

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(&hx8389b_tx_buf, hx8389b_display_off_cmds,
			ARRAY_SIZE(hx8389b_display_off_cmds));

	pr_debug("mipi_hx8389b_lcd_off X\n");
	return 0;
}

static ssize_t mipi_hx8389b_wta_bl_ctrl(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int err;

	err =  kstrtoint(buf, 0, &mipi_hx8389b_bl_ctrl);
	if (err)
		return ret;

	pr_info("%s: bl ctrl set to %d\n", __func__, mipi_hx8389b_bl_ctrl);

	return ret;
}

static DEVICE_ATTR(bl_ctrl, S_IWUSR, NULL, mipi_hx8389b_wta_bl_ctrl);

static struct attribute *mipi_hx8389b_fs_attrs[] = {
	&dev_attr_bl_ctrl.attr,
	NULL,
};

static struct attribute_group mipi_hx8389b_fs_attr_group = {
	.attrs = mipi_hx8389b_fs_attrs,
};

static int mipi_hx8389b_create_sysfs(struct platform_device *pdev)
{
	int rc;
	struct msm_fb_data_type *mfd = platform_get_drvdata(pdev);

	if (!mfd) {
		pr_err("%s: mfd not found\n", __func__);
		return -ENODEV;
	}
	if (!mfd->fbi) {
		pr_err("%s: mfd->fbi not found\n", __func__);
		return -ENODEV;
	}
	if (!mfd->fbi->dev) {
		pr_err("%s: mfd->fbi->dev not found\n", __func__);
		return -ENODEV;
	}
	rc = sysfs_create_group(&mfd->fbi->dev->kobj,
		&mipi_hx8389b_fs_attr_group);
	if (rc) {
		pr_err("%s: sysfs group creation failed, rc=%d\n",
			__func__, rc);
		return rc;
	}

	return 0;
}

static int __devinit mipi_hx8389b_lcd_probe(struct platform_device *pdev)
{
	struct platform_device *pthisdev = NULL;
	pr_debug("%s\n", __func__);

	if (pdev->id == 0) {
		mipi_hx8389b_pdata = pdev->dev.platform_data;
		if (mipi_hx8389b_pdata->bl_lock)
			spin_lock_init(&mipi_hx8389b_pdata->bl_spinlock);

                /* SKUD use PWM as backlight control method */
                if(machine_is_msm8625q_skud()) {
                        mipi_hx8389b_bl_ctrl = 1;
                }

		return 0;
	}

	pthisdev = msm_fb_add_device(pdev);
	mipi_hx8389b_create_sysfs(pthisdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_hx8389b_lcd_probe,
	.driver = {
		.name   = "mipi_hx8389b",
	},
};

static int old_bl_level;

static void mipi_hx8389b_set_backlight(struct msm_fb_data_type *mfd)
{
	int bl_level;
	unsigned long flags;
	bl_level = mfd->bl_level;

	if (mipi_hx8389b_pdata->bl_lock) {
		if (!mipi_hx8389b_bl_ctrl) {
			/* Level received is of range 1 to bl_max,
			   We need to convert the levels to 1
			   to 31 */
			bl_level = (2 * bl_level * 31 + mfd->panel_info.bl_max)
					/(2 * mfd->panel_info.bl_max);
			if (bl_level == old_bl_level)
				return;

			if (bl_level == 0)
				mipi_hx8389b_pdata->backlight(0, 1);

			if (old_bl_level == 0)
				mipi_hx8389b_pdata->backlight(50, 1);

			spin_lock_irqsave(&mipi_hx8389b_pdata->bl_spinlock,
						flags);
			mipi_hx8389b_pdata->backlight(bl_level, 0);
			spin_unlock_irqrestore(&mipi_hx8389b_pdata->bl_spinlock,
						flags);
			old_bl_level = bl_level;
		} else {
			mipi_hx8389b_pdata->backlight(bl_level, 1);
		}
	} else {
		mipi_hx8389b_pdata->backlight(bl_level, mipi_hx8389b_bl_ctrl);
	}
}

static struct msm_fb_panel_data hx8389b_panel_data = {
	.on	= mipi_hx8389b_lcd_on,
	.off = mipi_hx8389b_lcd_off,
	.set_backlight = mipi_hx8389b_set_backlight,
};

static int ch_used[3];

static int mipi_hx8389b_lcd_init(void)
{
	mipi_dsi_buf_alloc(&hx8389b_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&hx8389b_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}
int mipi_hx8389b_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	ret = mipi_hx8389b_lcd_init();
	if (ret) {
		pr_err("mipi_hx8389b_lcd_init() failed with ret %u\n", ret);
		return ret;
	}

	pdev = platform_device_alloc("mipi_hx8389b", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	hx8389b_panel_data.panel_info = *pinfo;
	ret = platform_device_add_data(pdev, &hx8389b_panel_data,
				sizeof(hx8389b_panel_data));
	if (ret) {
		pr_debug("%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		pr_debug("%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}
