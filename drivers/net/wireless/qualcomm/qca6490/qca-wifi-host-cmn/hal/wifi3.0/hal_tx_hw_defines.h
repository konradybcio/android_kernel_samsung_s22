/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _HAL_TX_HW_DEFINES_H_
#define _HAL_TX_HW_DEFINES_H_

#define HAL_TX_MSDU_EXTENSION_TSO_ENABLE_OFFSET		0x00000000
#define HAL_TX_MSDU_EXTENSION_TSO_ENABLE_LSB		0
#define HAL_TX_MSDU_EXTENSION_TSO_ENABLE_MASK		0x00000001

#define HAL_TX_MSDU_EXTENSION_TCP_FLAG_OFFSET		0x00000000
#define HAL_TX_MSDU_EXTENSION_TCP_FLAG_LSB		7
#define HAL_TX_MSDU_EXTENSION_TCP_FLAG_MASK		0x0000ff80

#define HAL_TX_MSDU_EXTENSION_TCP_FLAG_MASK_OFFSET	0x00000000
#define HAL_TX_MSDU_EXTENSION_TCP_FLAG_MASK_LSB		16
#define HAL_TX_MSDU_EXTENSION_TCP_FLAG_MASK_MASK	0x01ff0000

#define HAL_TX_MSDU_EXTENSION_L2_LENGTH_OFFSET		0x00000004
#define HAL_TX_MSDU_EXTENSION_L2_LENGTH_LSB		0
#define HAL_TX_MSDU_EXTENSION_L2_LENGTH_MASK		0x0000ffff

#define HAL_TX_MSDU_EXTENSION_IP_LENGTH_OFFSET		0x00000004
#define HAL_TX_MSDU_EXTENSION_IP_LENGTH_LSB		16
#define HAL_TX_MSDU_EXTENSION_IP_LENGTH_MASK		0xffff0000

#define HAL_TX_MSDU_EXTENSION_TCP_SEQ_NUMBER_OFFSET	0x00000008
#define HAL_TX_MSDU_EXTENSION_TCP_SEQ_NUMBER_LSB	0
#define HAL_TX_MSDU_EXTENSION_TCP_SEQ_NUMBER_MASK	0xffffffff

#define HAL_TX_MSDU_EXTENSION_IP_IDENTIFICATION_OFFSET	0x0000000c
#define HAL_TX_MSDU_EXTENSION_IP_IDENTIFICATION_LSB	0
#define HAL_TX_MSDU_EXTENSION_IP_IDENTIFICATION_MASK	0x0000ffff

#define HAL_TX_MSDU_EXTENSION_BUF0_PTR_31_0_OFFSET	0x00000018
#define HAL_TX_MSDU_EXTENSION_BUF0_PTR_31_0_LSB		0
#define HAL_TX_MSDU_EXTENSION_BUF0_PTR_31_0_MASK	0xffffffff

#define HAL_TX_MSDU_EXTENSION_BUF0_PTR_39_32_OFFSET	0x0000001c
#define HAL_TX_MSDU_EXTENSION_BUF0_PTR_39_32_LSB	0
#define HAL_TX_MSDU_EXTENSION_BUF0_PTR_39_32_MASK	0x000000ff

#define HAL_TX_MSDU_EXTENSION_BUF0_LEN_OFFSET		0x0000001c
#define HAL_TX_MSDU_EXTENSION_BUF0_LEN_LSB		16
#define HAL_TX_MSDU_EXTENSION_BUF0_LEN_MASK		0xffff0000

#define HAL_TX_MSDU_EXTENSION_BUF1_PTR_31_0_OFFSET	0x00000020
#define HAL_TX_MSDU_EXTENSION_BUF1_PTR_31_0_LSB		0
#define HAL_TX_MSDU_EXTENSION_BUF1_PTR_31_0_MASK	0xffffffff

#define HAL_TX_MSDU_EXTENSION_BUF1_PTR_39_32_OFFSET	0x00000024
#define HAL_TX_MSDU_EXTENSION_BUF1_PTR_39_32_LSB	0
#define HAL_TX_MSDU_EXTENSION_BUF1_PTR_39_32_MASK	0x000000ff

#define HAL_TX_MSDU_EXTENSION_BUF1_LEN_OFFSET		0x00000024
#define HAL_TX_MSDU_EXTENSION_BUF1_LEN_LSB		16
#define HAL_TX_MSDU_EXTENSION_BUF1_LEN_MASK		0xffff0000

#define HAL_TX_MSDU_EXTENSION_BUF2_PTR_31_0_OFFSET	0x00000028
#define HAL_TX_MSDU_EXTENSION_BUF2_PTR_31_0_LSB		0
#define HAL_TX_MSDU_EXTENSION_BUF2_PTR_31_0_MASK	0xffffffff

#define HAL_TX_MSDU_EXTENSION_BUF2_PTR_39_32_OFFSET	0x0000002c
#define HAL_TX_MSDU_EXTENSION_BUF2_PTR_39_32_LSB	0
#define HAL_TX_MSDU_EXTENSION_BUF2_PTR_39_32_MASK	0x000000ff

#define HAL_TX_MSDU_EXTENSION_BUF2_LEN_OFFSET		0x0000002c
#define HAL_TX_MSDU_EXTENSION_BUF2_LEN_LSB		16
#define HAL_TX_MSDU_EXTENSION_BUF2_LEN_MASK		0xffff0000

#define HAL_TX_MSDU_EXTENSION_BUF3_PTR_31_0_OFFSET	0x00000030
#define HAL_TX_MSDU_EXTENSION_BUF3_PTR_31_0_LSB		0
#define HAL_TX_MSDU_EXTENSION_BUF3_PTR_31_0_MASK	0xffffffff

#define HAL_TX_MSDU_EXTENSION_BUF3_PTR_39_32_OFFSET	0x00000034
#define HAL_TX_MSDU_EXTENSION_BUF3_PTR_39_32_LSB	0
#define HAL_TX_MSDU_EXTENSION_BUF3_PTR_39_32_MASK	0x000000ff

#define HAL_TX_MSDU_EXTENSION_BUF3_LEN_OFFSET		0x00000034
#define HAL_TX_MSDU_EXTENSION_BUF3_LEN_LSB		16
#define HAL_TX_MSDU_EXTENSION_BUF3_LEN_MASK		0xffff0000

#define HAL_TX_MSDU_EXTENSION_BUF4_PTR_31_0_OFFSET	0x00000038
#define HAL_TX_MSDU_EXTENSION_BUF4_PTR_31_0_LSB		0
#define HAL_TX_MSDU_EXTENSION_BUF4_PTR_31_0_MASK	0xffffffff

#define HAL_TX_MSDU_EXTENSION_BUF4_PTR_39_32_OFFSET	0x0000003c
#define HAL_TX_MSDU_EXTENSION_BUF4_PTR_39_32_LSB	0
#define HAL_TX_MSDU_EXTENSION_BUF4_PTR_39_32_MASK	0x000000ff

#define HAL_TX_MSDU_EXTENSION_BUF4_LEN_OFFSET		0x0000003c
#define HAL_TX_MSDU_EXTENSION_BUF4_LEN_LSB		16
#define HAL_TX_MSDU_EXTENSION_BUF4_LEN_MASK		0xffff0000

#define HAL_TX_MSDU_EXTENSION_BUF5_PTR_31_0_OFFSET	0x00000040
#define HAL_TX_MSDU_EXTENSION_BUF5_PTR_31_0_LSB		0
#define HAL_TX_MSDU_EXTENSION_BUF5_PTR_31_0_MASK	0xffffffff

#define HAL_TX_MSDU_EXTENSION_BUF5_PTR_39_32_OFFSET	0x00000044
#define HAL_TX_MSDU_EXTENSION_BUF5_PTR_39_32_LSB	0
#define HAL_TX_MSDU_EXTENSION_BUF5_PTR_39_32_MASK	0x000000ff

#define HAL_TX_MSDU_EXTENSION_BUF5_LEN_OFFSET		0x00000044
#define HAL_TX_MSDU_EXTENSION_BUF5_LEN_LSB		16
#define HAL_TX_MSDU_EXTENSION_BUF5_LEN_MASK		0xffff0000

/* TX completion ring MACROS */
#define HAL_TX_COMP_TX_RATE_STATS_OFFSET   0x00000014
#define HAL_TX_COMP_TX_RATE_STATS_LSB      0
#define HAL_TX_COMP_TX_RATE_STATS_MASK     0xffffffff

#define HAL_TX_COMP_SW_PEER_ID_OFFSET			0x1c
#define HAL_TX_COMP_SW_PEER_ID_LSB			0
#define HAL_TX_COMP_SW_PEER_ID_MASK			0x0000ffff

#define HAL_TX_COMP_BUFFER_OR_DESC_TYPE_OFFSET		0x8
#define HAL_TX_COMP_BUFFER_OR_DESC_TYPE_LSB		0x6
#define HAL_TX_COMP_BUFFER_OR_DESC_TYPE_MASK		0x000001c0

#define HAL_TX_COMP_TQM_RELEASE_REASON_OFFSET		0x8
#define HAL_TX_COMP_TQM_RELEASE_REASON_LSB		13
#define HAL_TX_COMP_TQM_RELEASE_REASON_MASK		0x0001e000

#define HAL_WBM2SW_RING_RELEASE_SOURCE_MODULE_OFFSET	0x8
#define HAL_WBM2SW_RING_RELEASE_SOURCE_MODULE_LSB	0
#define HAL_WBM2SW_RING_RELEASE_SOURCE_MODULE_MASK	0x00000007

#endif /* _HAL_TX_HW_DEFINES_H_ */
