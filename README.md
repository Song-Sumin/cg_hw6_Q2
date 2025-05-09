# cg_hw6_Q2 readme

## What you need
You need Visual Studio 2022 and window 11 OS.

And C/C++ should be available in VS2022.

## About
This project is about Flat Shading.

To view the result image, open Q1_result.png, 

and if you want an explanation of the code, scroll down below.

## How to run

1. Click code and download as zip file.
   
![image](https://github.com/user-attachments/assets/63aa9597-8679-4c43-b6d4-450faa6a89dd)


3. Unzip a download file

![image](https://github.com/user-attachments/assets/a5d0b5b4-ca2e-476f-927b-c7776b0d996f)



3. Open hw2_Q1-master. Double click hw2_Q1-master and opne OpenglViewer.sln

![image](https://github.com/user-attachments/assets/167dea26-bd07-4600-8694-6e796fed85f1)



5. click "F5" on your keybord. Then you will get the result.

![image](https://github.com/user-attachments/assets/3ccf0a60-e349-4a03-99b6-688749ccf6b9)



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
// Flat shading: Use the normal at the centroid of the triangle
void rasterize_triangle(vec4 v0, vec4 v1, vec4 v2, vec3 n0, vec3 n1, vec3 n2) {

   ...

    // Calculate the centroid and use its normal for flat shading
    vec3 centroid = (p0 + p1 + p2) / 3.0f;
    vec3 normal = normalize((n0 + n1 + n2) / 3.0f);

   ...

            if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0)) {
                // Barycentric interpolation  ---->>>추가
                float area = std::abs((s1.x - s0.x) * (s2.y - s0.y) - (s2.x - s0.x) * (s1.y - s0.y));
                float w0 = std::abs((s1.x - p.x) * (s2.y - p.y) - (s2.x - p.x) * (s1.y - p.y)) / area;
                float w1 = std::abs((s2.x - p.x) * (s0.y - p.y) - (s0.x - p.x) * (s2.y - p.y)) / area;
                float w2 = 1.0f - w0 - w1;

                // Depth interpolation  ---->>>추가
                float depth = w0 * p0.z + w1 * p1.z + w2 * p2.z;
                int idx = y * WIDTH + x;
                if (depth < DepthBuffer[idx]) {
                    DepthBuffer[idx] = depth;

                    // Flat shading uses the centroid normal  ---->>>추가
                    vec3 color = compute_phong_lighting(centroid, normal);

                    OutputImage[3 * idx + 0] = color.r;
                    OutputImage[3 * idx + 1] = color.g;
                    OutputImage[3 * idx + 2] = color.b;
                }
            }
        }
    }
}

```
![image](https://github.com/user-attachments/assets/0f137e36-9023-4d87-a415-8ed7ffa1a465)

Flat shading is implemented using the per-triangle normal.

vec3 color = compute_phong_lighting(centroid, normal); <- this code 

For each pixel inside the triangle, compute_phong_lighting is called to compute the shaded color.

-----------
```
void render_scene() 내부 추가
        // Calculate per-triangle normals
        vec3 n0 = normalize(cross(vec3(gVertices[gIndexBuffer[i + 1]]) - vec3(gVertices[gIndexBuffer[i]]),
            vec3(gVertices[gIndexBuffer[i + 2]]) - vec3(gVertices[gIndexBuffer[i]])));

        // Rasterize the triangle with flat shading
        rasterize_triangle(v0, v1, v2, n0, n0, n0);
```
Computes the flat normal (per-triangle normal) by taking the cross product of two edges of the triangle.

Additional parameters were added to the rasterize_triangle

--------------
