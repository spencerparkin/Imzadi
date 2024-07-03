{
    "constants_buffer": "Shaders/DebugLine.constants_buffer",
    "ps_entry_point": "PS_Main",
    "ps_model": "ps_5_0",
    "ps_shader_object": "Shaders/DebugLinePS.dxbc",
    "shader_code": "Shaders/DebugLine.hlsl",
    "vs_entry_point": "VS_Main",
    "vs_input_layout": [
        {
            "element_format": "R32G32B32_FLOAT",
            "semantic": "POS",
            "semantic_index": 0,
            "slot": 0
        },
        {
            "element_format": "R32G32B32_FLOAT",
            "semantic": "COL",
            "semantic_index": 0,
            "slot": 0
        }
    ],
    "vs_model": "vs_5_0",
    "vs_shader_object": "Shaders/DebugLineVS.dxbc",
    "rasterizer_state": {
        "fill_mode": "FILL_SOLID",
        "cull_mode": "CULL_BACK",
        "front_ccw": true,
        "depth_clip_enabled": true
    },
    "depth_stencil_state": {
        "depth_enabled": true,
        "stencil_enabled": false
    },
    "blend_state": {
        "blend_enabled": true,
        "src_blend": "BLEND_SRC_ALPHA",
        "dest_blend": "BLEND_INV_SRC_ALPHA",
        "blend_op": "BLEND_OP_ADD",
        "src_blend_alpha": "BLEND_ONE",
        "dest_blend_alpha": "BLEND_ZERO",
        "blend_op_alpha": "BLEND_OP_ADD"
    }
}