/**
  ******************************************************************************
  * @file    network.h
  * @author  STEdgeAI
  * @date    2026-05-18 02:27:21
  * @brief   Minimal description of the generated c-implemention of the network
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
#ifndef LL_ATON_NETWORK_H
#define LL_ATON_NETWORK_H

/******************************************************************************/
#define LL_ATON_NETWORK_C_MODEL_NAME        "network"
#define LL_ATON_NETWORK_ORIGIN_MODEL_NAME   "tomato_plant_yolov8n_320_best_int8_s8_qdq"

/************************** USER ALLOCATED IOs ********************************/
// No user allocated inputs
// No user allocated outputs

/************************** INPUTS ********************************************/
#define LL_ATON_NETWORK_IN_NUM        (1)    // Total number of input buffers
// Input buffer 1 -- Input_2_out_0
#define LL_ATON_NETWORK_IN_1_ALIGNMENT   (32)
#define LL_ATON_NETWORK_IN_1_SIZE_BYTES  (307200)

/************************** OUTPUTS *******************************************/
#define LL_ATON_NETWORK_OUT_NUM        (1)    // Total number of output buffers
// Output buffer 1 -- Concat_553_out_0
#define LL_ATON_NETWORK_OUT_1_ALIGNMENT   (32)
#define LL_ATON_NETWORK_OUT_1_SIZE_BYTES  (42000)

#endif /* LL_ATON_NETWORK_H */
