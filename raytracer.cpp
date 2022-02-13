#include <iostream>
#include <vector>
#include <math.h>
#include <float.h>
#include <fstream>

const int IMAGE_WIDTH = 1440, IMAGE_HEIGHT = 1080;
const float INF = FLT_MAX; // Infinity is set to max possible int value (32bits)
const float PI = 3.1415926535897931;

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {};
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};

    float length() {
        return sqrtf(x*x + y*y + z*z);
    }
    Vec3& normalize() {
        if (length() > 0) {
            float inverseL = 1 / length();
            x *= inverseL;
            y *= inverseL;
            z *= inverseL;  
        }   
        return *this;
    } 
    float dot(Vec3 v) {
        return x * v.x + y * v.y + z * v.z;
    }
    Vec3 vecAddition(Vec3 v) {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }
    Vec3 vecSubtraction(Vec3 v) {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }
    Vec3 scalarMultiply(float f) {
        return Vec3(f * x, f * y, f * z);
    }
    void scalarAddition(float f) {
        x += f;
        y += f;
        z += f;
        //return Vec3(x + f, y + f, z + f);
    }
};

struct Material {
    //
};

struct Sphere {
    Vec3 center;
    float radius;
    Vec3 color; // Colours are represented as 3Vectors, [R, G, B]
    
    Sphere(Vec3 _center, float _radius, Vec3 _color) : center(_center), radius(_radius), color(_color) {}
};

struct Light {
    Vec3 position;
    float intensity;

    Light(Vec3 _position, float _intensity, Vec3 _color = Vec3(1,1,1)) : position(_position), intensity(_intensity) {}
};

bool checkIntersection(Vec3 origin, Vec3 direction, Sphere &s, float &t0, float &t1) {
    // https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing
    Vec3 l = s.center.vecSubtraction(origin);
    float tca = direction.dot(l);
    if (tca < 0) return false;
    float d2 = sqrtf(l.dot(l) - tca * tca);
    if (d2 > s.radius * s.radius) return false;
    float thc = sqrtf(s.radius * s.radius - d2);
    t0 = tca - thc;
    t1 = tca + thc;
    if (t0 <= 0) return false;
    return true;
}

Vec3 trace(Vec3 origin, Vec3 direction, std::vector<Sphere>& objects, std::vector<Light>& lights) {
    float tnearest = INF;
    Sphere *hit = NULL;
    float t0 = INF, t1 = INF;
    for (int i = 0; i < objects.size(); i++) {
        if (checkIntersection(origin, direction, objects[i], t0, t1)) {
            if (t0 > 0 && t0 < tnearest) { 
                tnearest = t0; 
                hit = &objects[i]; 
            } 
        }
    }
    if (hit == NULL) return Vec3(1,1,1); // background color

    float diffuse_intensity = 0; 
    Vec3 pint = origin.vecAddition(direction.scalarMultiply(tnearest));
    Vec3 pnorm = pint.vecSubtraction(hit->center).normalize();

    // Diffuse Objects
    for (int i = 0; i < lights.size(); i++) {
        Vec3 light_dir = lights[i].position.vecSubtraction(pint).normalize();
        diffuse_intensity += std::max(float(0), pnorm.dot(light_dir)) * lights[i].intensity;
        // for (int j = 0; j < objects.size(); j++) {
        //     if (i != j) {
        //         float t0, t1;
        //         if (checkIntersection(pint, dir, objects[j], t0, t1)) {
        //             cont = Vec3(0,0,0);
        //             break;
        //         }
        //     }
        // }
    }
    return hit->color.scalarMultiply(diffuse_intensity);
}

void render(std::vector<Sphere>& objects, std::vector<Light>& lights) {
    std::vector<Vec3> img_buf(IMAGE_WIDTH*IMAGE_HEIGHT);
    Vec3 origin;
    for (int i = 0; i < IMAGE_HEIGHT; i++) {
        for (int j = 0; j < IMAGE_WIDTH; j++) {
            float x =  (2*(j + 0.5)/(float)IMAGE_WIDTH  - 1) * (IMAGE_WIDTH / (float)IMAGE_HEIGHT);
            float y = -(2*(i + 0.5)/(float)IMAGE_HEIGHT - 1);
            Vec3 primaryRay(x, y, -1);
            img_buf[i*IMAGE_WIDTH + j] = trace(origin, primaryRay.normalize(), objects, lights);
        }
    }

    // https://www.scratchapixel.com/code.php?id=3&origin=/lessons/3d-basic-rendering/introduction-to-ray-tracing
    std::ofstream ofs("./output.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << IMAGE_WIDTH << " " << IMAGE_HEIGHT << "\n255\n";
    for (unsigned i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; ++i) { 
        ofs << (unsigned char)(std::min(float(1), img_buf[i].x) * 255) << 
               (unsigned char)(std::min(float(1), img_buf[i].y) * 255) << 
               (unsigned char)(std::min(float(1), img_buf[i].z) * 255); 
    } 
    ofs.close(); 
}

int main() { 
    std::vector<Sphere> spheres;
    std::vector<Light> lights;
    spheres.push_back(Sphere(Vec3(0,0,-10), 1.2, Vec3(.1,.3,.3)));
    spheres.push_back(Sphere(Vec3(-1,-1,-3), 0.5, Vec3(.3,.1,.1)));
    lights.push_back(Light(Vec3(10, 10, 10), 3));

    render(spheres, lights);
}