//
//  Created by Bradley Austin Davis on 2016/05/14
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "GLShared.h"

#include <mutex>
#include <fstream>

#include <QtCore/QThread>

#include <gl/GLHelpers.h>
#include <GPUIdent.h>
#include <NumericalConstants.h>

Q_LOGGING_CATEGORY(gpugllogging, "hifi.gpu.gl")
Q_LOGGING_CATEGORY(trace_render_gpu_gl, "trace.render.gpu.gl")
Q_LOGGING_CATEGORY(trace_render_gpu_gl_detail, "trace.render.gpu.gl.detail")

namespace gpu { namespace gl {

gpu::Size getFreeDedicatedMemory() {
    Size result { 0 };
#if !defined(USE_GLES)
    static bool nvidiaMemorySupported { true };
    static bool atiMemorySupported { true };
    if (nvidiaMemorySupported) {
        
        GLint nvGpuMemory { 0 };
        glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &nvGpuMemory);
        if (GL_NO_ERROR == glGetError()) {
            result = KB_TO_BYTES(nvGpuMemory);
        } else {
            nvidiaMemorySupported = false;
        }
    } else if (atiMemorySupported) {
        GLint atiGpuMemory[4];
        // not really total memory, but close enough if called early enough in the application lifecycle
        glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, atiGpuMemory);
        if (GL_NO_ERROR == glGetError()) {
            result = KB_TO_BYTES(atiGpuMemory[0]);
        } else {
            atiMemorySupported = false;
        }
    }
#endif
    return result;
}

ComparisonFunction comparisonFuncFromGL(GLenum func) {
    if (func == GL_NEVER) {
        return NEVER;
    } else if (func == GL_LESS) {
        return LESS;
    } else if (func == GL_EQUAL) {
        return EQUAL;
    } else if (func == GL_LEQUAL) {
        return LESS_EQUAL;
    } else if (func == GL_GREATER) {
        return GREATER;
    } else if (func == GL_NOTEQUAL) {
        return NOT_EQUAL;
    } else if (func == GL_GEQUAL) {
        return GREATER_EQUAL;
    } else if (func == GL_ALWAYS) {
        return ALWAYS;
    }

    return ALWAYS;
}

State::StencilOp stencilOpFromGL(GLenum stencilOp) {
    if (stencilOp == GL_KEEP) {
        return State::STENCIL_OP_KEEP;
    } else if (stencilOp == GL_ZERO) {
        return State::STENCIL_OP_ZERO;
    } else if (stencilOp == GL_REPLACE) {
        return State::STENCIL_OP_REPLACE;
    } else if (stencilOp == GL_INCR_WRAP) {
        return State::STENCIL_OP_INCR_SAT;
    } else if (stencilOp == GL_DECR_WRAP) {
        return State::STENCIL_OP_DECR_SAT;
    } else if (stencilOp == GL_INVERT) {
        return State::STENCIL_OP_INVERT;
    } else if (stencilOp == GL_INCR) {
        return State::STENCIL_OP_INCR;
    } else if (stencilOp == GL_DECR) {
        return State::STENCIL_OP_DECR;
    }

    return State::STENCIL_OP_KEEP;
}

State::BlendOp blendOpFromGL(GLenum blendOp) {
    if (blendOp == GL_FUNC_ADD) {
        return State::BLEND_OP_ADD;
    } else if (blendOp == GL_FUNC_SUBTRACT) {
        return State::BLEND_OP_SUBTRACT;
    } else if (blendOp == GL_FUNC_REVERSE_SUBTRACT) {
        return State::BLEND_OP_REV_SUBTRACT;
    } else if (blendOp == GL_MIN) {
        return State::BLEND_OP_MIN;
    } else if (blendOp == GL_MAX) {
        return State::BLEND_OP_MAX;
    }

    return State::BLEND_OP_ADD;
}

State::BlendArg blendArgFromGL(GLenum blendArg) {
    if (blendArg == GL_ZERO) {
        return State::ZERO;
    } else if (blendArg == GL_ONE) {
        return State::ONE;
    } else if (blendArg == GL_SRC_COLOR) {
        return State::SRC_COLOR;
    } else if (blendArg == GL_ONE_MINUS_SRC_COLOR) {
        return State::INV_SRC_COLOR;
    } else if (blendArg == GL_DST_COLOR) {
        return State::DEST_COLOR;
    } else if (blendArg == GL_ONE_MINUS_DST_COLOR) {
        return State::INV_DEST_COLOR;
    } else if (blendArg == GL_SRC_ALPHA) {
        return State::SRC_ALPHA;
    } else if (blendArg == GL_ONE_MINUS_SRC_ALPHA) {
        return State::INV_SRC_ALPHA;
    } else if (blendArg == GL_DST_ALPHA) {
        return State::DEST_ALPHA;
    } else if (blendArg == GL_ONE_MINUS_DST_ALPHA) {
        return State::INV_DEST_ALPHA;
    } else if (blendArg == GL_CONSTANT_COLOR) {
        return State::FACTOR_COLOR;
    } else if (blendArg == GL_ONE_MINUS_CONSTANT_COLOR) {
        return State::INV_FACTOR_COLOR;
    } else if (blendArg == GL_CONSTANT_ALPHA) {
        return State::FACTOR_ALPHA;
    } else if (blendArg == GL_ONE_MINUS_CONSTANT_ALPHA) {
        return State::INV_FACTOR_ALPHA;
    }

    return State::ONE;
}

void getCurrentGLState(State::Data& state) {
    {
#if defined(USE_GLES)
        state.fillMode = State::FILL_FACE;
#else
        GLint modes[2];
        glGetIntegerv(GL_POLYGON_MODE, modes);
        if (modes[0] == GL_FILL) {
            state.fillMode = State::FILL_FACE;
        } else {
            if (modes[0] == GL_LINE) {
                state.fillMode = State::FILL_LINE;
            } else {
                state.fillMode = State::FILL_POINT;
            }
        }
#endif
    }
    {
        if (glIsEnabled(GL_CULL_FACE)) {
            GLint mode;
            glGetIntegerv(GL_CULL_FACE_MODE, &mode);
            state.cullMode = (mode == GL_FRONT ? State::CULL_FRONT : State::CULL_BACK);
        } else {
            state.cullMode = State::CULL_NONE;
        }
    }
    {
        GLint winding;
        glGetIntegerv(GL_FRONT_FACE, &winding);
        state.flags.frontFaceClockwise = (winding == GL_CW);
#if defined(USE_GLES)
        state.flags.multisampleEnable = glIsEnabled(GL_MULTISAMPLE_EXT);
        state.flags.antialisedLineEnable = false;
        state.flags.depthClampEnable = false;
#else
        state.flags.multisampleEnable = glIsEnabled(GL_MULTISAMPLE);
        state.flags.antialisedLineEnable = glIsEnabled(GL_LINE_SMOOTH);
        state.flags.depthClampEnable = glIsEnabled(GL_DEPTH_CLAMP);
#endif
        state.flags.scissorEnable = glIsEnabled(GL_SCISSOR_TEST);
    }
    {
        if (glIsEnabled(GL_POLYGON_OFFSET_FILL)) {
            glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &state.depthBiasSlopeScale);
            glGetFloatv(GL_POLYGON_OFFSET_UNITS, &state.depthBias);
        }
    }
    {
        GLboolean isEnabled = glIsEnabled(GL_DEPTH_TEST);
        GLboolean writeMask;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &writeMask);
        GLint func;
        glGetIntegerv(GL_DEPTH_FUNC, &func);

        state.depthTest = State::DepthTest(isEnabled, writeMask, comparisonFuncFromGL(func));
    }
    {
        GLboolean isEnabled = glIsEnabled(GL_STENCIL_TEST);

        GLint frontWriteMask;
        GLint frontReadMask;
        GLint frontRef;
        GLint frontFail;
        GLint frontDepthFail;
        GLint frontPass;
        GLint frontFunc;
        glGetIntegerv(GL_STENCIL_WRITEMASK, &frontWriteMask);
        glGetIntegerv(GL_STENCIL_VALUE_MASK, &frontReadMask);
        glGetIntegerv(GL_STENCIL_REF, &frontRef);
        glGetIntegerv(GL_STENCIL_FAIL, &frontFail);
        glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &frontDepthFail);
        glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &frontPass);
        glGetIntegerv(GL_STENCIL_FUNC, &frontFunc);

        GLint backWriteMask;
        GLint backReadMask;
        GLint backRef;
        GLint backFail;
        GLint backDepthFail;
        GLint backPass;
        GLint backFunc;
        glGetIntegerv(GL_STENCIL_BACK_WRITEMASK, &backWriteMask);
        glGetIntegerv(GL_STENCIL_BACK_VALUE_MASK, &backReadMask);
        glGetIntegerv(GL_STENCIL_BACK_REF, &backRef);
        glGetIntegerv(GL_STENCIL_BACK_FAIL, &backFail);
        glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_FAIL, &backDepthFail);
        glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_PASS, &backPass);
        glGetIntegerv(GL_STENCIL_BACK_FUNC, &backFunc);

        state.stencilActivation = State::StencilActivation(isEnabled, frontWriteMask, backWriteMask);
        state.stencilTestFront = State::StencilTest(frontRef, frontReadMask, comparisonFuncFromGL(frontFunc), stencilOpFromGL(frontFail), stencilOpFromGL(frontDepthFail), stencilOpFromGL(frontPass));
        state.stencilTestBack = State::StencilTest(backRef, backReadMask, comparisonFuncFromGL(backFunc), stencilOpFromGL(backFail), stencilOpFromGL(backDepthFail), stencilOpFromGL(backPass));
    }
    {
        GLint mask = 0xFFFFFFFF;
        if (glIsEnabled(GL_SAMPLE_MASK)) {
            glGetIntegerv(GL_SAMPLE_MASK, &mask);
            state.sampleMask = mask;
        }
        state.sampleMask = mask;
    }
    {
        state.flags.alphaToCoverageEnable = glIsEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE);
    }
    {
        GLboolean isEnabled = glIsEnabled(GL_BLEND);
        GLint srcRGB;
        GLint srcA;
        GLint dstRGB;
        GLint dstA;
        glGetIntegerv(GL_BLEND_SRC_RGB, &srcRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcA);
        glGetIntegerv(GL_BLEND_DST_RGB, &dstRGB);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &dstA);

        GLint opRGB;
        GLint opA;
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &opRGB);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &opA);

        state.blendFunction = State::BlendFunction(isEnabled,
            blendArgFromGL(srcRGB), blendOpFromGL(opRGB), blendArgFromGL(dstRGB),
            blendArgFromGL(srcA), blendOpFromGL(opA), blendArgFromGL(dstA));
    }
    {
        GLboolean mask[4];
        glGetBooleanv(GL_COLOR_WRITEMASK, mask);
        state.colorWriteMask = (State::ColorMask)((mask[0] ? State::WRITE_RED : 0)
            | (mask[1] ? State::WRITE_GREEN : 0)
            | (mask[2] ? State::WRITE_BLUE : 0)
            | (mask[3] ? State::WRITE_ALPHA : 0));
    }

    (void)CHECK_GL_ERROR();
}


void serverWait() {
    auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    assert(fence);
    glWaitSync(fence, 0, GL_TIMEOUT_IGNORED);
    glDeleteSync(fence);
}

void clientWait() {
    auto fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    assert(fence);
    auto result = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
    while (GL_TIMEOUT_EXPIRED == result || GL_WAIT_FAILED == result) {
        // Minimum sleep
        QThread::usleep(1);
        result = glClientWaitSync(fence, 0, 0);
    }
    glDeleteSync(fence);
}

} }


using namespace gpu;


