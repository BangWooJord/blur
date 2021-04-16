#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>
#include <iomanip>
#include <random>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>


class simpleTimer {
private:
    std::chrono::time_point<std::chrono::steady_clock> start;
public:
    simpleTimer() {
        start = std::chrono::high_resolution_clock::now();
    }
    ~simpleTimer() {
        auto finish = std::chrono::high_resolution_clock::now();
        std::cout << "Time taken " << std::chrono::duration<float>(finish - start).count() << " s" << std::endl;
    }
};

void createMatrix(std::vector<std::vector<int>>& matrix, int w, int h) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 256);

    matrix.resize(w);
    for (int i = 0; i < w; ++i) {
        matrix[i].resize(h);
        for (int j = 0; j < h; ++j) {
            matrix[i][j] = dist(mt);
        }
    }
}

template <typename T>
void printMatrix(std::vector<std::vector<T>>& matrix) {
    for (uint32_t i = 0; i < matrix.size(); ++i) {
        for (uint32_t j = 0; j < matrix[i].size(); ++j) {
            std::cout << std::setw(5) << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

template <typename T>
void blur(std::vector<std::vector<T>> &matrix, int blur_radius)
{
    int h = matrix.size();
    int w = matrix[0].size();

    std::vector<std::vector<T>> result = matrix;

    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            int top_border = (i < blur_radius) ? i : (i-blur_radius);
            int left_border = (j < blur_radius) ? j : (j-blur_radius);
            int right_border = ((w - j) < blur_radius) ? w : (j + blur_radius);
            int bot_border = ((h - i) < blur_radius) ? h : (i + blur_radius);

            long sum = 0;
            int total_num = (bot_border - top_border) * (right_border - left_border);
            for (int k = top_border; k < bot_border; ++k) {
                for (int l = left_border; l < right_border; ++l) {
                    sum += matrix[k][l];
                }
            }
            result[i][j] = sum / total_num;
        }
    }

    matrix = result;
    /*uint32_t wsize = bottom_limit - top_limit;
    * 
    temp_matrix.resize(wsize);
    for (uint32_t i = 0; i < wsize; ++i) {
        temp_matrix[i].resize(h);
        for (uint32_t j = 0; j < h; ++j)
            temp_matrix[i][j] = 0;
    }
    int blur_border = (blur_radius - 1) / 2;
    if (top_limit >= blur_border) {
        for (uint32_t i = blur_border; i < bottom_limit - blur_border; ++i) {
            for (uint32_t j = blur_border; j < h - blur_border; ++j) {
                temp_matrix[i][j] = 0;
                for (uint32_t k = (-1) * blur_border; k < blur_border; ++k) {
                    for (uint32_t l = (-1) * blur_border; l < blur_border; ++l) {
                        temp_matrix[i][j] += matrix[top_limit + i + k][j + l];
                    }
                }
                temp_matrix[i][j] /= (blur_radius * blur_radius);
            }
        }
    }*/
}

template <typename T>
cv::Mat vecToCvMat(std::vector<std::vector<T>> &input_vec) {
    int rows = input_vec.size();
    int cols = input_vec[0].size();

    cv::Mat res(rows, cols, CV_8U);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            res.at<uchar>(i,j) = input_vec[i][j];
        }
    }
    return res;
}

int main() {
    std::string image_path;
    try {
        image_path = cv::samples::findFile("../image_input/cat.jpg", 1);
    }
    catch (std::exception e) {
        std::cerr << "Error caught: " << e.what() << std::endl;
    }

    cv::Mat image = cv::imread(image_path, cv::IMREAD_COLOR);
    auto image_ch = image.channels();

    cv::Mat *image_channels = new cv::Mat[image_ch];
    /*std::vector<cv::Mat> image_channels;
    image_channels.resize(image_ch);*/
    cv::split(image, image_channels);

    std::vector<cv::Mat> output;

    std::cout << "Input blur radius" << std::endl;
    int blur_radius;
    std::cin >> blur_radius;

    std::vector<std::vector<std::vector<int>>> img_matrix;
    img_matrix.resize(image_ch);
    int ch = 0;
    std::vector<std::thread> thread_vec;
    for (auto& new_matrix : img_matrix) {
        thread_vec.emplace_back(std::thread ([&new_matrix, &image_channels, blur_radius, ch]() {
            //simpleTimer st;
            new_matrix.resize(image_channels[ch].rows);
            for (int i = 0; i < image_channels[ch].rows; ++i) {
                new_matrix[i].resize(image_channels[ch].cols);
                for (int j = 0; j < image_channels[ch].cols; ++j) {
                    try {
                        new_matrix[i][j] = image_channels[ch].at<uchar>(i, j);
                    }
                    catch (std::exception err) {
                        std::cerr << "Error caught: " << err.what() << std::endl;
                    }
                }
            }

            try {
                blur(new_matrix, blur_radius);
            }
            catch (std::exception err) {
                std::cerr << "Exception in blur: " << err.what() << std::endl;
            }
        }));
        ++ch;
    }
    int i = 0;
    for (auto& vec : thread_vec) {
        vec.join();
        output.push_back(vecToCvMat(img_matrix[i]));
        ++i;
    }
    cv::Mat result;
    cv::merge(output, result);

    try {
        cv::namedWindow("Blur?");
        cv::imshow("Blur?", result);
        cv::waitKey(0);
    }
    catch (cv::Exception e) {
        std::cerr << e.what() << std::endl;
    }

    /*try {
        cv::imwrite("../image_output/Tank.jpg", result);
    }
    catch (cv::Exception err) {
        std::cerr << "Exception caught: " << err.what() << std::endl;
    }*/

    return 0;
}
