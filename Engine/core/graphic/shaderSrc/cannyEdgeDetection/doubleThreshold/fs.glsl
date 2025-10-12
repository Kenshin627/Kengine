#version 460 core

uniform sampler2D nmsMap;
uniform float uHighThreshold; // 高阈值
uniform float uLowThreshold;  // 低阈值

in vec2 vTexcoord;
out vec4 FragColor;

vec2 texelSize = 1.0 / textureSize(nmsMap, 0);

// 检查相邻像素是否有强边缘
bool hasStrongNeighbor() {
    for(int y = -1; y <= 1; y++) {
        for(int x = -1; x <= 1; x++) {
            if(x == 0 && y == 0) continue; // 跳过当前像素
            
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            float neighborValue = texture(nmsMap, vTexcoord + offset).r;
            
            if(neighborValue >= uHighThreshold) {
                return true;
            }
        }
    }
    return false;
}

void main() {
    float value = texture(nmsMap, vTexcoord).r;
    
    //强边缘：保留
    if(value >= uHighThreshold) {
        FragColor = vec4(1.0); // 白色表示强边缘
    }
    //弱边缘：检查是否与强边缘相连
    else if(value >= uLowThreshold) {
        if(hasStrongNeighbor()) {
            FragColor = vec4(1.0); //与强边缘相连的弱边缘保留
        } else {
            FragColor = vec4(0.0); //孤立的弱边缘抑制
        }
    }
    // 低于低阈值：抑制
    else {
        FragColor = vec4(0.0);
    }
}
    