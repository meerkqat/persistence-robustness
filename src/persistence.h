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

typedef struct {
    float dim;
    float birth;
    float death;
    QString cycle;
} PersistenceItem;

// Wrapper...
class Persistence
{
public:
    Persistence() {}

    bool set_in_file(QString pts);
    void set_distance(double distance)            { distance_ = distance; }

    const PointList& get_pts()             { return pts_; }

    bool calculate();

private:
    void calc_rips_(QString filename);
    QVector<double> calcHomology(QVector<QVector3D> persistence, double dist);
    bool shakeDataset();

    PointList orig_pts_;
    PointList pts_;
    // in_file
    QString in_file;
    // max+distance. if 0, then rips.max_distance()
    double distance_ = 7.5;
    // max dimension of a generator
    Dimension skeleton_ = 3;
    // "true" random if set to 0 (i.e. takes time as seed)
    int rand_seed_ = 0;
    // how much the points should be "shaken" i.e. eps = max_distance*eps_factor
    double eps_ = -1;
    double eps_factor_ = 0.01;
    // do the calculation on the original dataset and this many shaken datasets
    int num_shaken_datasets_ = 30;//100;
    // how many slices should [0,R] be divided into (delta param)
    int num_slices_ = 11;
    // cut of lifetime, if shorter
    float min_lifetime_ = 0.05;
};

#endif // PERSISTENCE_H
