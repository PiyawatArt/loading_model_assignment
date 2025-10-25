#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 objectColor;
uniform int useTexture;

void main()
{
    if (useTexture == 1) {
        // ใช้ texture สำหรับรถ
        FragColor = texture(texture_diffuse1, TexCoords);
    } else {
        // ใช้สีเรียบสำหรับพื้น/ลูกบอล
        float ambientStrength = 0.5;
        vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
        
        vec3 lightPos = vec3(50.0, 100.0, 50.0);
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
        
        vec3 result = (ambient + diffuse) * objectColor;
        FragColor = vec4(result, 1.0);
    }
}