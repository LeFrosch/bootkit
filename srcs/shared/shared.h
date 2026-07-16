#pragma once

#define as_wide_str_(str) L##str
#define as_wide_str(str) as_wide_str_(str)

#define BK_LOPT_DESCRIPTION "bk_loader"
#define BK_LOPT_DESCRIPTION_W as_wide_str(BK_LOPT_DESCRIPTION)

