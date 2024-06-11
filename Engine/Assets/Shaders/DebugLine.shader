{
    "constants": {
        "worldToProjection": {
            "offset": 0,
            "size": 64,
            "type": "float"
        }
    },
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
    "vs_shader_object": "Shaders/DebugLineVS.dxbc"
}