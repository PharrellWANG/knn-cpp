/* kdtree_flann.h
 *
 *     Author: Fabian Meyer
 * Created On: 09 Nov 2018
 */

#ifndef KDT_KDTREE_FLANN_H_
#define KDT_KDTREE_FLANN_H_

#include <Eigen/Geometry>
#include <flann/flann.hpp>

namespace kdt
{
    template<typename Scalar,
        typename Distance=flann::L2_Simple<Scalar>>
    class KDTreeFlann
    {
    public:
        typedef Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> Matrix;
        typedef Eigen::Matrix<Scalar, Eigen::Dynamic, 1> Vector;

    private:
        Matrix dataCopy_;
        flann::Matrix<Scalar> data_;

        flann::Index<Distance> index_;
        flann::SearchParams params_;

    public:
        KDTreeFlann()
            : dataCopy_(), data_(nullptr, 0, 0), index_(flann::KDTreeSingleIndexParams())
        {

        }

        KDTreeFlann(Matrix &data, const bool copy = false)
            : KDTreeFlann()
        {
            setData(data, copy);
        }

        void setIndexParams(const flann::IndexParams &params)
        {
             index_ = flann::Index<Distance>(params);
        }

        void setLeafVisits(const int visits)
        {
            params_.checks = visits;
        }

        void setSorted(const bool sorted)
        {
            params_.sorted = sorted;
        }

        void setThreads(const int threads)
        {
            params_.cores = threads;
        }

        void setEpsilon(const float eps)
        {
            params_.eps = eps;
        }

        void setData(Matrix &data, const bool copy = false)
        {
            if(copy)
            {
                dataCopy_ = data;
                // switch rows and columns
                // flann uses row major; eigen uses column major
                data_ = flann::Matrix<Scalar>(dataCopy_.data(),
                    dataCopy_.cols(), dataCopy_.rows());
            }
            else
            {
                // switch rows and columns
                // flann uses row major; eigen uses column major
                data_ = flann::Matrix<Scalar>(data.data(), data.cols(),
                    data.rows());
            }
        }

        void build()
        {
            if(data_.ptr() == nullptr)
                throw std::runtime_error("cannot build KDTree; data not set");

            index_.buildIndex(data_);
        }

        int query(Matrix &points, const size_t knn, Eigen::MatrixXi &indices,
                Matrix &distances) const
        {
            if(index_.size() == 0)
                throw std::runtime_error("cannot query KDTree; not built yet");
            if(static_cast<Eigen::Index>(index_.veclen()) != points.rows())
                throw std::runtime_error("cannot query KDTree; index has different dimension than query data");

            distances.resize(knn, points.cols());
            indices.resize(knn, points.cols());

            const flann::Matrix<Scalar> pointsF(points.data(), points.cols(),
                points.rows());
            flann::Matrix<int> indicesF(indices.data(), indices.cols(),
                indices.rows());
            flann::Matrix<Scalar> distancesF(distances.data(), distances.cols(),
                distances.rows());

            return index_.knnSearch(pointsF, indicesF, distancesF, knn,
                params_);
        }

        void clear()
        {
            index_ = flann::Index<Distance>(index_.getParameters());
        }
    };

    typedef KDTreeFlann<double> KDTreeFlannd;
    typedef KDTreeFlann<float> KDTreeFlannf;
}

#endif
