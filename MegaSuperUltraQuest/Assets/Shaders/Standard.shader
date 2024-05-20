{
    "shader_code": "Shaders/Standard.hlsl",
    "vs_entry_point": "VS_Main",
    "ps_entry_point": "PS_Main",
    "vs_model": "vs_5_0",
    "ps_model": "ps_5_0",
    "vs_input_layout": [
        {
            "semantic": "POS",
            "semantic_index": 0,
            "element_format": "R32G32B32_FLOAT",
            "slot": 0
        },
        {
            "semantic": "TEX",
            "semantic_index": 0,
            "element_format": "R32G32_FLOAT",
            "slot": 0
        },
        {
            "semantic": "NORM",
            "semantic_index": 0,
            "element_format": "R32G32B32_FLOAT",
            "slot": 0
        }
    ],
    "constants": {
        "object_to_projection": {
            "offset": 0,
            "size": 64,
            "type": "float"
        },
        "color": {
            "offset": 64,
            "size": 16,
            "type": "float"
        },
        "light_direction": {
            "offset": 80,
            "size": 12,
            "type": "float"
        },
        "light_intensity": {
            "offset": 92,
            "size": 4,
            "type": "float"
        },
        "light_color": {
            "offset": 96,
            "size": 16,
            "type": "float"
        }
    }
}