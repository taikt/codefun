/**
* \file    start_main.cpp
* \brief     This is entry point to start Service layer.
*
* \details
*    This software is copyright protected and proprietary to
*    LG electronics. LGE grants to you only those rights as
*    set out in the license conditions. All other rights remain
*    with LG electronics.
* \author       sungwoo.oh
* \date       2015.03.18
* \attention Copyright (c) 2015 by LG electronics co, Ltd. All rights reserved.
*/
#include "daemon/start/startd/leopard.h"

int32_t main(int32_t argc, char **argv)
{
    /* launch start process titled leopard. */
    Leopard::launch();
}
