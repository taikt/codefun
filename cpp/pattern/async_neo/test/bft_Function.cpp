/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/Function.h>

using namespace nf;

#ifdef NF_BFT_0

void CannotCopyAssign()
{
    Function<void()> f1;
    Function<void()> f2;
    f2 = f1;
}

#endif

#ifdef NF_BFT_1

void CannotCopyConstruct()
{
    Function<void()> f1;
    Function<void()> f2 = f1;
}

#endif

#ifdef NF_BFT_2

void CannotCopyConstruct()
{
    Function<void()> f1;
    Function<void()> f2{f1};
}

#endif
