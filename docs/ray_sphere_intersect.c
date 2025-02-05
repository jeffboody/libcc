// https://www.perplexity.ai

#include <math.h>

typedef struct { float p[3]; float v[3]; } ray_t;
typedef struct { float c[3]; float r; } sphere_t;

int ray_sphere_intersect(ray_t* ray, sphere_t* sphere, float* near, float* far) {
    float oc[3] = {ray->p[0] - sphere->c[0], ray->p[1] - sphere->c[1], ray->p[2] - sphere->c[2]};
    float a = ray->v[0]*ray->v[0] + ray->v[1]*ray->v[1] + ray->v[2]*ray->v[2];
    float b = 2.0f * (oc[0]*ray->v[0] + oc[1]*ray->v[1] + oc[2]*ray->v[2]);
    float c = oc[0]*oc[0] + oc[1]*oc[1] + oc[2]*oc[2] - sphere->r*sphere->r;

    float discriminant = b*b - 4*a*c;

    if (discriminant < 0) {
        return 0; // No intersection
    }

    float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
    float t2 = (-b + sqrt(discriminant)) / (2.0f * a);

    if (t1 > t2) {
        float temp = t1;
        t1 = t2;
        t2 = temp;
    }

    if (t1 < 0) {
        if (t2 < 0) {
            return 0; // Both intersection points are behind the ray origin
        }
        *near = 0; // Ray origin is inside the sphere
        *far = t2;
        return 1;
    }

    *near = t1;
    *far = t2;
    return 2; // Ray intersects the sphere twice
}
