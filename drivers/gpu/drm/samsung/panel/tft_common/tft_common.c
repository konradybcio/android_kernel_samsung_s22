/*
 * linux/drivers/video/fbdev/exynos/panel/tft_common/tft_common.c
 *
 * TFT_COMMON Dimming Driver
 *
 * Copyright (c) 2016 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/of_gpio.h>
#include <video/mipi_display.h>
#include "../panel.h"
#include "tft_common.h"

#include "../panel_drv.h"
#include "../panel_debug.h"

#if IS_ENABLED(CONFIG_EXYNOS_DECON_LCD_TFT_M33)
#include "./m33/tft_common_m33_panel_list.h"
#endif

#if IS_ENABLED(CONFIG_EXYNOS_DECON_LCD_TFT_GTS8L)
#include "./gts8l/tft_common_gts8l_panel_list.h"
#endif

#ifdef PANEL_PR_TAG
#undef PANEL_PR_TAG
#define PANEL_PR_TAG	"ddi"
#endif

static int generate_brt_step_table(struct brightness_table *brt_tbl)
{
	int ret = 0;
	int i = 0, j = 0, k = 0;

	if (unlikely(!brt_tbl || !brt_tbl->brt)) {
		panel_err("invalid parameter\n");
		return -EINVAL;
	}
	if (unlikely(!brt_tbl->step_cnt)) {
		if (likely(brt_tbl->brt_to_step)) {
			panel_info("we use static step table\n");
			return ret;
		}
		panel_err("invalid parameter, all table is NULL\n");
		return -EINVAL;
	}

	brt_tbl->sz_brt_to_step = 0;
	for (i = 0; i < brt_tbl->sz_step_cnt; i++)
		brt_tbl->sz_brt_to_step += brt_tbl->step_cnt[i];

	brt_tbl->brt_to_step =
		kmalloc_array(brt_tbl->sz_brt_to_step, sizeof(u32), GFP_KERNEL);

	if (unlikely(!brt_tbl->brt_to_step)) {
		panel_err("alloc fail\n");
		return -EINVAL;
	}
	brt_tbl->brt_to_step[0] = brt_tbl->brt[0];
	i = 1;
	while (i < brt_tbl->sz_brt_to_step) {
		for (k = 1; k < brt_tbl->sz_brt; k++) {
			for (j = 1; j <= brt_tbl->step_cnt[k]; j++, i++) {
				brt_tbl->brt_to_step[i] = interpolation(brt_tbl->brt[k - 1] * disp_pow(10, 2),
					brt_tbl->brt[k] * disp_pow(10, 2), j, brt_tbl->step_cnt[k]);
				brt_tbl->brt_to_step[i] = disp_pow_round(brt_tbl->brt_to_step[i], 2);
				brt_tbl->brt_to_step[i] = disp_div64(brt_tbl->brt_to_step[i], disp_pow(10, 2));
				if (brt_tbl->brt[brt_tbl->sz_brt - 1] < brt_tbl->brt_to_step[i])
					brt_tbl->brt_to_step[i] = disp_pow_round(brt_tbl->brt_to_step[i], 2);

				if (i >= brt_tbl->sz_brt_to_step) {
					panel_err("step cnt over %d %d\n", i, brt_tbl->sz_brt_to_step);
					break;
				}
			}
		}
	}
	return ret;
}

int init_common_table(struct maptbl *tbl)
{
	if (tbl == NULL) {
		panel_err("maptbl is null\n");
		return -EINVAL;
	}

	if (tbl->pdata == NULL) {
		panel_err("pdata is null\n");
		return -EINVAL;
	}

	return 0;
}

static int init_brt_table(struct maptbl *tbl)
{
	struct panel_info *panel_data;
	struct panel_device *panel;
	struct panel_dimming_info *panel_dim_info;
	//todo:remove
	panel_info("++\n");
	if (tbl == NULL) {
		panel_err("maptbl is null\n");
		return -EINVAL;
	}

	if (tbl->pdata == NULL) {
		panel_err("pdata is null\n");
		return -EINVAL;
	}

	panel = tbl->pdata;
	panel_data = &panel->panel_data;

	panel_dim_info = panel_data->panel_dim_info[PANEL_BL_SUBDEV_TYPE_DISP];

	if (panel_dim_info == NULL) {
		panel_err("panel_dim_info is null\n");
		return -EINVAL;
	}

	if (panel_dim_info->brt_tbl == NULL) {
		panel_err("panel_dim_info->brt_tbl is null\n");
		return -EINVAL;
	}

	generate_brt_step_table(panel_dim_info->brt_tbl);

	/* initialize brightness_table */
	memcpy(&panel->panel_bl.subdev[PANEL_BL_SUBDEV_TYPE_DISP].brt_tbl,
			panel_dim_info->brt_tbl, sizeof(struct brightness_table));

	return 0;
}

static int getidx_brt_table(struct maptbl *tbl)
{
	int row = 0;
	struct panel_info *panel_data;
	struct panel_bl_device *panel_bl;
	struct panel_device *panel = (struct panel_device *)tbl->pdata;

	if (panel == NULL) {
		panel_err("panel is null\n");
		return -EINVAL;
	}
	panel_bl = &panel->panel_bl;
	panel_data = &panel->panel_data;

	row = get_brightness_pac_step(panel_bl, panel_bl->props.brightness);

	return maptbl_index(tbl, 0, row, 0);
}

static void copy_common_maptbl(struct maptbl *tbl, u8 *dst)
{
	int idx;

	if (!tbl || !dst) {
		panel_err("invalid parameter (tbl %pK, dst %pK)\n",
				tbl, dst);
		return;
	}

	idx = maptbl_getidx(tbl);
	if (idx < 0) {
		panel_err("failed to getidx %d\n", idx);
		return;
	}

	memcpy(dst, &(tbl->arr)[idx], sizeof(u8) * maptbl_get_sizeof_copy(tbl));
	panel_dbg("copy from %s %d %d\n",
			tbl->name, idx, maptbl_get_sizeof_copy(tbl));
	print_data(dst, maptbl_get_sizeof_copy(tbl));
}

__visible_for_testing int __init tft_common_panel_init(void)
{
	register_common_panel_list();

	return 0;
}

__visible_for_testing void __exit tft_common_panel_exit(void)
{
	unregister_common_panel_list();
}

module_init(tft_common_panel_init)
module_exit(tft_common_panel_exit)
MODULE_DESCRIPTION("Samsung Mobile Panel Driver");
MODULE_LICENSE("GPL");
