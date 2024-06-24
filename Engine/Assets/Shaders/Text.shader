{
    "constants": {
        "objectToProjection": {
            "offset": 0,
            "size": 64,
            "type": "float"
        },
        "textColor": {
            "offset": 64,
            "size": 12,
            "type": "float"
        }
    },
    "ps_entry_point": "PS_Main",
    "ps_model": "ps_5_0",
    "ps_shader_object": "Shaders/TextPS.dxbc",
    "shader_code": "Shaders/Text.hlsl",
    "vs_entry_point": "VS_Main",
    "vs_input_layout": [
        {
            "element_format": "R32G32B32_FLOAT",
            "semantic": "POS",
            "semantic_index": 0,
            "slot": 0
        },
        {
            "element_format": "R32G32_FLOAT",
            "semantic": "TEX",
            "semantic_index": 0,
            "slot": 0
        }
    ],
    "vs_model": "vs_5_0",
    "vs_shader_object": "Shaders/TextVS.dxbc"
}