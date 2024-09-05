{
    "constants": {
        "ambientLightIntensity": {
            "offset": 144,
            "size": 4,
            "type": "float"
        },
        "cameraEyePoint": {
            "offset": 160,
            "size": 12,
            "type": "float"
        },
        "directionalLightIntensity": {
            "offset": 140,
            "size": 4,
            "type": "float"
        },
        "lightCameraEyePoint": {
            "offset": 176,
            "size": 12,
            "type": "float"
        },
        "lightCameraFar": {
            "offset": 232,
            "size": 4,
            "type": "float"
        },
        "lightCameraHeight": {
            "offset": 224,
            "size": 4,
            "type": "float"
        },
        "lightCameraNear": {
            "offset": 228,
            "size": 4,
            "type": "float"
        },
        "lightCameraWidth": {
            "offset": 220,
            "size": 4,
            "type": "float"
        },
        "lightCameraXAxis": {
            "offset": 192,
            "size": 12,
            "type": "float"
        },
        "lightCameraYAxis": {
            "offset": 208,
            "size": 12,
            "type": "float"
        },
        "lightDirection": {
            "offset": 128,
            "size": 12,
            "type": "float"
        },
        "objectToProjection": {
            "offset": 0,
            "size": 64,
            "type": "float"
        },
        "objectToWorld": {
            "offset": 64,
            "size": 64,
            "type": "float"
        },
        "shadowScale": {
            "offset": 236,
            "size": 4,
            "type": "float"
        },
        "shininessExponent": {
            "offset": 148,
            "size": 4,
            "type": "float"
        }
    },
    "ps_entry_point": "PS_Main",
    "ps_model": "ps_5_0",
    "ps_shader_object": "Shaders/StandardPS.dxbc",
    "shader_code": "Shaders/Standard.hlsl",
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
        },
        {
            "element_format": "R32G32B32_FLOAT",
            "semantic": "NORM",
            "semantic_index": 0,
            "slot": 0
        }
    ],
    "vs_model": "vs_5_0",
    "vs_shader_object": "Shaders/StandardVS.dxbc"
}