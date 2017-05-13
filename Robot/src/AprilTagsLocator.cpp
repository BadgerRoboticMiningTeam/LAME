#include "AprilTagsLocator.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include "tag36h11.h"
#include "common/image_u8.h"
#include "common/zarray.h"

using namespace LAME;

constexpr int DEFAULT_ALIGNMENT = 96;
constexpr int MAX_TAGS = 3;


AprilTagsLocator::AprilTagsLocator(float hfov, float tag_size)
{
    this->hfov = hfov;
    this->tag_size = tag_size;
    this->tf = tag36h11_create();
    this->td = apriltag_detector_create();
    
    apriltag_detector_add_family(this->td, this->tf);
}

AprilTagsLocator::~AprilTagsLocator()
{
    if (td)
        apriltag_detector_destroy(td);
    if (tf)
        tag36h11_destroy(tf);
}

bool AprilTagsLocator::GetSphericalFromMat(cv::Mat& mat, std::vector<TagDetection>& tags)
{
    int height;
    int width;
    int stride;
    cv::Mat filtered;
    cv::Mat compensate;
    cv::Mat adjusted_stride_image;
    zarray_t *detections = nullptr;
    int detection_size;

    if (!mat.data)
        return false;

    // convert image to next multiple of DEFAULT_ALIGNMENT for AT //
    cv::cvtColor(mat, filtered, cv::COLOR_BGR2GRAY);
    stride = mat.cols;
    stride += DEFAULT_ALIGNMENT - (stride % DEFAULT_ALIGNMENT);

    compensate = cv::Mat::zeros(filtered.rows, stride - filtered.cols, CV_8UC1);
    cv::hconcat(filtered, compensate, adjusted_stride_image);
    
    height = adjusted_stride_image.rows;
    width = adjusted_stride_image.cols;

    // create PNM //
    image_u8_t im = {
        width,
        height,
        stride, 
        (uchar *)(adjusted_stride_image.ptr(0))
    };

    detections = apriltag_detector_detect(this->td, &im);
    detection_size = zarray_size(detections);
    if (detection_size == 0)
    {
        apriltag_detections_destroy(detections);
        return false;
    }

    float half_fov = this->hfov / 2.0;
    float x_center = width / 2.0;
    float y_center = height / 2.0;

    for (int i = 0; i < detection_size; i++)
    {
        apriltag_detection_t *det = nullptr;
        TagDetection info;
        float short_half;
        float long_half;
        float long_angle;
        float pixel_distance;
        float a;
        float b;
        float pixel_tag_size;
        bool swapped = false;

        zarray_get(detections, i, &det);

        // length in pixels from center of the detection to two opposide corners
        // ratio of the two lengths is used to compensate for tag rotation in 3d space
        short_half = sqrtf(powf(det->p[0][0] - det->c[0], 2) + powf(det->p[0][1] - det->c[1], 2));
        long_half = sqrtf(powf(det->p[2][0] - det->c[0], 2) + powf(det->p[2][1] - det->c[1], 2));

        if (short_half > long_half)
        {
            swapped = true;
            std::swap(short_half, long_half);
        }

        // calculate distance from camera
        // compensates for 3d rotation of tag by comparing short_half and long_half
        long_angle = atanf((long_half / x_center) * tanf(half_fov));
        pixel_distance = long_half / tanf(long_angle);
        a = pixel_distance * ((long_half - short_half) / (long_half + short_half));
        b = (long_half/pixel_distance) * (pixel_distance - a);
        pixel_tag_size = sqrtf(powf(a, 2) + powf(b, 2));

        // azimuth and elevation reported in radians from center of camera view
        // length unit matches unit of tag_size
        info.azimuth = atanf(((det->c[0] - x_center) / x_center) * tanf(half_fov));
        info.elevation = atanf(((y_center - det->c[1]) / x_center) * tanf(half_fov));
        info.distance = pixel_distance * 0.5 * (tag_size / pixel_tag_size);
        info.rotation = (swapped) ? -atanf(a / b) : atanf(a / b);

        // copy over det data
        info.id = det->id;
        info.center = { det->c[0], det->c[1] };
        for (int i = 0; i < 4; i++)
            info.corners.push_back({ det->p[i][0], det->p[i][1] });
        
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                info.homography(i, j) = matd_get(det->H, i, j);

        tags.push_back(info);
    }
    apriltag_detections_destroy(detections);
    return true;
}
