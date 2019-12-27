# 
#  Created by Bradley Austin Davis on 2018/07/22
#  Copyright 2013-2018 High Fidelity, Inc.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_JSON)
	# We use vcpkg for both json and glm, so we just re-use the target_glm macro here
    target_glm()
endmacro()