#version 460 core

uniform sampler2D uGradientTexture; // r:幅度, g:方向

in vec2 vTexcoord;
out vec4 FragColor;

// 将角度转换为8个可能的梯度方向之一
int getGradientDirection(float angle) {
    // 将角度标准化到0~π（因为梯度方向是双向的）
    angle = mod(angle, 3.1415926);
    
    // 定义8个方向的阈值（弧度）
    float thresholds[8] = float[8](
        22.5 * 3.1415926 / 180.0,   // 0: 0°左右
        67.5 * 3.1415926 / 180.0,   // 1: 45°左右
        112.5 * 3.1415926 / 180.0,  // 2: 90°左右
        157.5 * 3.1415926 / 180.0,  // 3: 135°左右
        202.5 * 3.1415926 / 180.0,  // 4: 180°左右
        247.5 * 3.1415926 / 180.0,  // 5: 225°左右
        292.5 * 3.1415926 / 180.0,  // 6: 270°左右
        337.5 * 3.1415926 / 180.0   // 7: 315°左右
    );
    
    // 确定当前角度属于哪个方向区间
    for(int i = 0; i < 8; i++) {
        if(angle < thresholds[i]) {
            return i;
        }
    }
    return 0;
}

void main() {
    vec2 texelSize = 1.0 / textureSize(uGradientTexture, 0);
    
    // 获取当前像素的梯度幅度和方向
    vec4 gradient = texture(uGradientTexture, vTexcoord);
    float currentMagnitude = gradient.r;
    float currentDirection = gradient.g;
    
    // 如果当前梯度幅度为0，直接输出黑色
    if(currentMagnitude <= 0.0) {
        FragColor = vec4(0.0);
        return;
    }
    
    // 确定梯度方向（8个可能方向之一）
    int dir = getGradientDirection(currentDirection);
    
    // 根据梯度方向确定需要比较的两个相邻像素
    vec2 neighbor1Coord = vTexcoord;
    vec2 neighbor2Coord = vTexcoord;
    
    // 根据方向设置相邻像素坐标
    switch(dir) {
        case 0:  // 0°左右（水平方向）
            neighbor1Coord.x -= texelSize.x;
            neighbor2Coord.x += texelSize.x;
            break;
        case 1:  // 45°左右（右上-左下）
            neighbor1Coord += vec2(-texelSize.x, -texelSize.y);
            neighbor2Coord += vec2(texelSize.x, texelSize.y);
            break;
        case 2:  // 90°左右（垂直方向）
            neighbor1Coord.y -= texelSize.y;
            neighbor2Coord.y += texelSize.y;
            break;
        case 3:  // 135°左右（左上-右下）
            neighbor1Coord += vec2(-texelSize.x, texelSize.y);
            neighbor2Coord += vec2(texelSize.x, -texelSize.y);
            break;
        case 4:  // 180°左右（与0°相反）
            neighbor1Coord.x += texelSize.x;
            neighbor2Coord.x -= texelSize.x;
            break;
        case 5:  // 225°左右（与45°相反）
            neighbor1Coord += vec2(texelSize.x, texelSize.y);
            neighbor2Coord += vec2(-texelSize.x, -texelSize.y);
            break;
        case 6:  // 270°左右（与90°相反）
            neighbor1Coord.y += texelSize.y;
            neighbor2Coord.y -= texelSize.y;
            break;
        case 7:  // 315°左右（与135°相反）
            neighbor1Coord += vec2(texelSize.x, -texelSize.y);
            neighbor2Coord += vec2(-texelSize.x, texelSize.y);
            break;
    }
    
    // 获取相邻像素的梯度幅度
    float neighbor1Magnitude = texture(uGradientTexture, neighbor1Coord).r;
    float neighbor2Magnitude = texture(uGradientTexture, neighbor2Coord).r;
    
    // 非极大值抑制
    if(currentMagnitude >= neighbor1Magnitude && currentMagnitude >= neighbor2Magnitude) {
        FragColor = vec4(vec3(currentMagnitude), 1.0);
    } else {
        FragColor = vec4(0.0);
    }
}
    