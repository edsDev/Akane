#pragma once
#include "akane/math/math.h"
#include "akane/math/sampling.h"
#include "akane/ray.h"

namespace akane::shape
{
    /**
     * A disk that is placed horizontally, paralell to xy plane
     */
    class Disk
    {
    public:
        constexpr Disk(Vec3 center, float radius) : center_(center), radius_(radius)
        {
            AKANE_REQUIRE(radius > 0);
        }

        bool Intersect(const Ray& ray, float t_min, float t_max, IntersectionInfo& isect) const
            noexcept
        {
            // ray is paralell to the disk
            if (ray.d.Z() == 0)
            {
                return false;
            }

            // compute point P that ray hits at plane disk's plane
            float t = (center_.Z() - ray.o.Z()) / ray.d.Z();
            Vec3 P  = ray.o + t * ray.d;

            if (t < t_min || t > t_max)
            {
                return false;
            }

            // test if hit point is in the circle
            if ((P - center_).LengthSq() > radius_ * radius_)
            {
                return false;
            }

            isect.t     = t;
            isect.point = P;
            isect.ng    = {0, 0, -1}; // TODO: not (0, 0, 1)?
            isect.ns    = {0, 0, -1};
            isect.uv    = {0, 0}; // TODO:
            return true;
        }

        float Area() const noexcept
        {
            return 2.f * kPi * radius_;
        }

        void SamplePoint(const Point2f& u, Vec3& p_out, Vec3& n_out, float& pdf_out) const noexcept
        {
            p_out   = SampleUniformDisk(u) * radius_ + center_;
            n_out   = {0, 0, -1};
            pdf_out = 1.f / Area();
        }

    private:
        Vec3 center_;
        float radius_;
    };
} // namespace akane::shape