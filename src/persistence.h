#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <array>
#include <qstring.h>

#include <topology/simplex.h>
#include <topology/filtration.h>


#include <QVector>
#include <QVector3D>

typedef std::array<double, 3> TPoint;

typedef std::vector<TPoint> PointList;

// Wrapper...
class Persistence
{
public:
    Persistence(): distance_(1) {}

    bool set_in_file(QString pts);
    void set_distance(double distance)            { distance_ = distance; }

    const PointList& get_pts()             { return pts_; }

    bool calculate();

private:
    QVector<QVector3D> calc_rips_(QString filename);
    QVector<double> calcHomology(QVector<QVector3D> persistence, double dist);
    bool shakeDataset();

    double distance_;
    PointList orig_pts_;
    PointList pts_;
    Dimension skeleton_ = 3;
    // "true" random if set to 0 (i.e. takes time as seed)
    int rand_seed_ = 42;
    // how much the points should be "shaken" i.e. eps = max_distance*eps_factor
    double eps_ = -1;
    double eps_factor_ = 0.01;
    // do the calculation on the original dataset and this many shaken datasets
    int num_shaken_datasets_ = 25;//100;
    // how many slices should [0,R] be divided into (delta param)
    int num_slices_ = 11;
};

#endif // PERSISTENCE_H
