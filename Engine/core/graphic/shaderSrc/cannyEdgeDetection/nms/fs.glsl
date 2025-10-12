#version 460 core

#define PI 3.14159265358979

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
    vec4 grad = texture(uGradientTexture, vTexcoord);
    float currentMagnitude = grad.r;
    float currentAngle = grad.g;

    currentAngle = mod(currentAngle, 2.0 * PI);
    if(currentAngle < 0.0)
    {
        currentAngle += 2.0 * PI;
    }

    int dir = 0;
    float tanVal = tan(currentAngle);
    if(abs(tanVal) <= 1.0)
    {
        dir = (tanVal >= 0.0)?2:3;
    }
    else
    {
        dir = tanVal > 0? 0:1;
    }

    vec2 neighhorOffset1, neighhorOffset2;
    if(dir == 0)
    {
        neighhorOffset1 = vec2(0.0, 1.0) * texelSize;
        neighhorOffset2 = vec2(0.0, -1.0) * texelSize;
    }
    else if(dir == 1)
    {
        neighhorOffset1 = vec2(1.0, 0.0) * texelSize;
        neighhorOffset2 = vec2(-1.0, 0.0) * texelSize;
    }
    else if(dir == 2)
    {
        neighhorOffset1 = vec2(1.0, 1.0) * texelSize;
        neighhorOffset2 = vec2(-1.0, -1.0) * texelSize;
    }
    else //dir == 3
    {
        neighhorOffset1 = vec2(1.0, -1.0) * texelSize;
        neighhorOffset2 = vec2(-1.0, 1.0) * texelSize;
    }
    
    
    float neighbor1Magnitude = texture(uGradientTexture, vTexcoord + neighhorOffset1).r;
    float neighbor2Magnitude = texture(uGradientTexture, vTexcoord + neighhorOffset2).r;
    
    // 非极大值抑制
    if(currentMagnitude >= neighbor1Magnitude && currentMagnitude >= neighbor2Magnitude) {
        FragColor = vec4(currentMagnitude, 0.0, 0.0, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
    