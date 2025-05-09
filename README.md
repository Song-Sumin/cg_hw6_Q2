# cg_hw6_Q2 readme

## What you need
You need Visual Studio 2022 and window 11 OS.

And C/C++ should be available in VS2022.

## About
This project is about Gouraud Shading.

To view the result image, open Q2_result.png, 

and if you want an explanation of the code, scroll down below.

## How to run

1. Click code and download as zip file.
   
![image](https://github.com/user-attachments/assets/6e03b17e-ee4c-472b-a456-0301e63ddad0)



2. Unzip a download file

![image](https://github.com/user-attachments/assets/cce0d53a-fb98-4cd3-8acc-bd15898e987b)



3. Open cg_hw6_Q2-master. Double click cg_hw6_Q2-master and opne OpenglViewer.sln

![image](https://github.com/user-attachments/assets/9466e7d8-84e5-4852-9403-da308e22baf5)



4. click "F5" on your keybord. Then you will get the result.

![image](https://github.com/user-attachments/assets/9cd21ee6-6d94-4880-9709-f9271e5b725c)

I couldn't find out why the result was little bit diffent. 

Looking closely, the light forms a square. So, I think Gouraud Shading work successful.

## Code explanation

I have only included explanations for the parts that were modified and added in HW5.

For other parts of the code, please refer to the HW5 README.

```
vec3 compute_phong_lighting(vec3 pos, vec3 normal) {
    // Material properties
    vec3 ka(0.0f, 1.0f, 0.0f);
    vec3 kd(0.0f, 0.5f, 0.0f);
    vec3 ks(0.5f, 0.5f, 0.5f);
    float p = 32.0f;

    // Light properties
    vec3 lightPos(4.0f, -4.0f, 3.0f);
    vec3 viewPos(0.0f, 0.0f, 0.0f);
    float Ia = 0.2f;

    // Lighting calculations
    vec3 L = normalize(lightPos - pos);
    vec3 V = normalize(viewPos - pos);
    vec3 R = reflect(-L, normal);

    vec3 ambient = Ia * ka;
    vec3 diffuse = kd * std::max(dot(normal, L), 0.0f);
    vec3 specular = ks * pow(std::max(dot(R, V), 0.0f), p);

    vec3 color = ambient + diffuse + specular;

    // Gamma correction
    color.r = pow(color.r, 1.0f / 2.2f);
    color.g = pow(color.g, 1.0f / 2.2f);
    color.b = pow(color.b, 1.0f / 2.2f);

    return clamp(color, 0.0f, 1.0f);
}
```
![image](https://github.com/user-attachments/assets/f4858440-856e-4c25-8725-80eb9d1d917e)

Implemented the compute_phong_lighting function to calculate lighting according to the instructions for HW6.

Only lightPos was hardcoded to fit the coordinate system of my code.

-------------


```
void rasterize_triangle_gouraud(vec4 v0, vec4 v1, vec4 v2, vec3 c0, vec3 c1, vec3 c2)

   ...

                float area = std::abs((s1.x - s0.x) * (s2.y - s0.y) - (s2.x - s0.x) * (s1.y - s0.y));
                float w0 = std::abs((s1.x - p.x) * (s2.y - p.y) - (s2.x - p.x) * (s1.y - p.y)) / area;
                float w1 = std::abs((s2.x - p.x) * (s0.y - p.y) - (s0.x - p.x) * (s2.y - p.y)) / area;
                float w2 = 1.0f - w0 - w1;

                float depth = w0 * p0.z + w1 * p1.z + w2 * p2.z;
                int idx = y * WIDTH + x;
               if (depth < DepthBuffer[idx]) {
                DepthBuffer[idx] = depth;
                vec3 color = w0 * c0 + w1 * c1 + w2 * c2;
                OutputImage[3 * idx + 0] = color.r;
                OutputImage[3 * idx + 1] = color.g;
                OutputImage[3 * idx + 2] = color.b;
               }
   ...

```
![image](https://github.com/user-attachments/assets/1790d620-bade-4b7b-bd4b-f293ac391fcd)


Gouraud Shading is implemented using per-vertex normal.

Interpolate the per-vertex normals using the barycentric weights w0, w1, and w2, and use the resulting normal to compute and output the final color.

-----------
```
void render_scene() {
   ...

    std::vector<vec3> VertexColors(gVertices.size());
    for (int i = 0; i < gVertices.size(); ++i) {
        vec3 pos = vec3(model * vec4(gVertices[i], 1.0f)); 
        vec3 normal = normalize(mat3(transpose(inverse(model))) * gVertices[i]); 
        VertexColors[i] = compute_phong_lighting(pos, normal);
    }

    for (int i = 0; i < gIndexBuffer.size(); i += 3) {
        int ia = gIndexBuffer[i];
        int ib = gIndexBuffer[i + 1];
        int ic = gIndexBuffer[i + 2];

        vec4 v0 = MVP * vec4(gVertices[ia], 1.0f);
        vec4 v1 = MVP * vec4(gVertices[ib], 1.0f);
        vec4 v2 = MVP * vec4(gVertices[ic], 1.0f);

        rasterize_triangle_gouraud(v0, v1, v2,
            VertexColors[ia], VertexColors[ib], VertexColors[ic]);
    }
}
```
![image](https://github.com/user-attachments/assets/4533f093-941d-4b6e-b57a-c417e698039b)

Computes the vertex normal.

Additional parameters were added to the rasterize_triangle

--------------
