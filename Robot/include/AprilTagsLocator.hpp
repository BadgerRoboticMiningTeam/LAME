#pragma once

#include <iostream>
#include <opencv2/core/core.hpp>
#include "apriltag.h"
#include <vector>

namespace LAME
{
    struct Vec2d
    {
        double x;
        double y;

        std::string toString()
        {
            std::string s;
            s.append("(");
            s.append(std::to_string(this->x));
            s.append(", ");
            s.append(std::to_string(this->y));
            s.append(")");
            return s;
        }
    };

    struct TagDetection
    {
        double azimuth;
        double elevation;
        double rotation;
        double distance;

        double id;
        Vec2d center;
        std::vector<Vec2d> corners;
        cv::Matx33d homography;

        void dump()
        {
            printf("Azimuth: %2.4f, Elevation: %2.4f\n", this->azimuth * 180 / M_PI, this->elevation);
            printf("Rotation: %2.4f, Distance: %4.2f\n", this->rotation * 180 / M_PI, this->distance);
            printf("Center: %s\n", this->center.toString().c_str());
            for (size_t j = 0; j < this->corners.size(); j++)
            {
                printf("Corner %zu: %s\n", j, this->corners[j].toString().c_str());
            }

            std::cout << "Homography: " << std::endl << this->homography << std::endl;
        }
    };

    class AprilTagsLocator
    {
        public:
            AprilTagsLocator(float hfov, float tag_size);
            ~AprilTagsLocator();
            bool GetSphericalFromMat(cv::Mat& mat, std::vector<TagDetection>& tags);
            
        private:
            float hfov;
            float tag_size;

            // AprilTags structs
            apriltag_family_t *tf;
            apriltag_detector_t *td;
    };
}
