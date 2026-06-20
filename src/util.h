#pragma once

#define ARRAY_SIZE(A) (sizeof(A) / sizeof(*A))

#define RAND(D)  (D * static_cast<double>(rand()) / RAND_MAX)
#define RANDF(F) (static_cast<float>(RAND(static_cast<double>(F))))
