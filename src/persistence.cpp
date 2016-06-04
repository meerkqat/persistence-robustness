#include <topology/simplex.h>
#include <topology/rips.h>
#include <topology/filtration.h>
#include <topology/static-persistence.h>
#include <topology/dynamic-persistence.h>
#include <topology/persistence-diagram.h>

#include <geometry/l2distance.h>
#include <geometry/distances.h>

#include <utilities/types.h>

#include <QVector3D>
#include <vector>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>

#include <qdebug.h>
#include <qtextstream.h>
#include <qfile.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string>

#include <sstream>
#include <iostream>
#include <fstream>

#include "persistence.h"

# define RAND_FLOAT (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));

typedef         PairwiseDistances<PointContainer, L2Distance>           PairDistances;
typedef         PairDistances::DistanceType                             DistanceType;
typedef         PairDistances::IndexType                                Vertex;

typedef         Rips<PairDistances>                                     Generator;
typedef         Generator::Simplex                                      Smplx;
typedef         Filtration<Smplx>                                       Fltr;
typedef         DynamicPersistenceChains<>                              DynamicPersistence;
typedef         PersistenceDiagram<>                                    PDgm;

bool Persistence::set_in_file(QString infile)
{
    orig_pts_.clear();
    QFile file(infile);

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug("Bad file!");
        return false;
    }

    double x_s=0, y_s=0, z_s=0;
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        // if line is comment or can't be at least "x y z"
        if(line[0] == '#' || line.size() < 5)
            continue;

        QStringList  fields = line.split(" ");

        double x = fields[0].toDouble();
        double y = fields[1].toDouble();
        double z = fields[2].toDouble();

        x_s += x; y_s += y; z_s += z;

        TPoint pt = {x, y, z};
        orig_pts_.push_back(pt);
    }

    x_s /= orig_pts_.size();
    y_s /= orig_pts_.size();
    z_s /= orig_pts_.size();

    for(uint i = 0; i < pts_.size(); i++)
    {
        orig_pts_[i][0] -= x_s;
        orig_pts_[i][1] -= y_s;
        orig_pts_[i][2] -= z_s;
    }

    in_file = infile;
    qDebug("Good file :)");
    return true;
}

bool Persistence::calculate()
{
    double prob_ = 1;
    pts_ = PointList(orig_pts_.begin(), orig_pts_.begin() + int(orig_pts_.size() * prob_));

    // calc the homology on the original dataset and n shaken datasets
    eps_ = -1;
    for (int j = 0; j <= num_shaken_datasets_; j++) {

        QString fname = "diagram_" + in_file +
                "_n" + QString::number(distance_) +
                "_m" + (max_distance ? "1" : "0") +
                "_p" + QString::number(prob_) +
                "_i" + QString::number(j) + ".txt";

        QVector<QVector3D> persistence = calc_rips_(fname);
        qDebug() << "Wrote to file: " << fname << "\n";

//        for (int i = 0; i < num_slices_; i++) {
//            qDebug() << "Distance " << (distance_/(num_slices_-1.0))*i;
//            qDebug() << calcHomology(persistence, (distance_/(num_slices_-1.0))*i);
//        }

        shakeDataset();
    }

    return true;
}

// move every point for +-eps_
bool Persistence::shakeDataset()
{
    srand(rand_seed_ == 0 ? time(NULL) : rand_seed_);

    for (uint i = 0; i < pts_.size(); i++) {
        // 2 * pi * [0.0,1.0]
        double phi = 4.0 * acos(0.0) * RAND_FLOAT;
        double theta = 4.0 * acos(0.0) * RAND_FLOAT;
        double r = eps_ * RAND_FLOAT;

        double x = r*sin(phi)*cos(theta);
        double y = r*sin(phi)*sin(theta);
        double z = r*cos(phi);

        pts_[i][0] = orig_pts_[i][0]+x;
        pts_[i][1] = orig_pts_[i][1]+y;
        pts_[i][2] = orig_pts_[i][2]+z;

    }

    return true;
}

double dummy_get_distance(const PointContainer& points)
{
    double max_dist = 0;
    double dist;
    for(uint i = 0; i < points.size(); i++)
        for(uint j = i + 1; j < points.size(); j++)
        {
            dist = (points[i][0] - points[j][0]) * (points[i][0] - points[j][0]) +
                   (points[i][1] - points[j][1]) * (points[i][1] - points[j][1]) +
                   (points[i][2] - points[j][2]) * (points[i][2] - points[j][2]);

            if(dist > max_dist)
                max_dist = dist;
        }
    return sqrt(max_dist);
}

QVector<QVector3D> Persistence::calc_rips_(QString filename)
{
    PointContainer points;

    for(auto iter = pts_.begin(); iter != pts_.end(); iter++)
    {
        points.push_back(Point());

        TPoint tp = *iter;
        //x = tp.at(0); y = tp.at(1); z = tp.at(2);

        points.back().push_back(tp.at(0));
        points.back().push_back(tp.at(1));
        points.back().push_back(tp.at(2));
    }

    PairDistances distances(points);
    Generator rips(distances);
    Generator::Evaluator size(distances);
    Fltr f;

    // Grabs max_distance from rips. Is correct (can check with dummy_max_distance
    if(max_distance)
    {
        distance_ = rips.max_distance();
    }

    qDebug() << "Distance: " << distance_;
    // distance_ = 2; // better run this in debug mode

    // first time through (original dataset) eps should be set
    if (eps_ == -1) {
        eps_ = distance_*eps_factor_;
    }

    // Generate n-skeleton of the Rips complex
    std::cout << "# Generating complex..." << std::endl;
    rips.generate(skeleton_, distance_, make_push_back_functor(f));
    std::cout << "# Generated complex of size: " << f.size() << std::endl;

    // Generate filtration with respect to distance and compute its persistence
    f.sort(Generator::Comparison(distances));

    DynamicPersistence p(f);
    p.pair_simplices();

    DynamicPersistence::SimplexMap<Fltr> m = p.make_simplex_map(f);

    // Output cycles
    QVector<QVector3D> persistence;
    for (DynamicPersistence::iterator cur = p.begin(); cur != p.end(); ++cur) {
        // only negative simplices have non-empty cycles
        if (!cur->sign()) {
            // the cycle that cur killed was born when we added birth (another simplex)
            DynamicPersistence::OrderIndex birth = cur->pair;

            const Smplx& b = m[birth];
            const Smplx& d = m[cur];

            if (b.dimension() >= skeleton_) continue;
            persistence.append({static_cast<float>(b.dimension()), static_cast<float>(size(b)), static_cast<float>(size(d))});
        }
        // positive could be unpaired
        else if (cur->unpaired()) {
            const Smplx& b = m[cur];
            if (b.dimension() >= skeleton_) continue;
            persistence.append({static_cast<float>(b.dimension()), static_cast<float>(size(b)), static_cast<float>(distance_ + 1)});
        }
    }

    QFile diagram_out(filename);

    if (diagram_out.open(QFile::ReadWrite | QFile::Text))
    {
        QTextStream out(&diagram_out);

        Q_FOREACH (auto line, persistence)
        {
            out << line.x() << " " << line.y() << " " << line.z() << "\n";
        }

        diagram_out.close();
    }
    else
    {
        qDebug() << "!! Cannot write to " << filename << " !!\n";
    }

    qDebug() << "Vietoris-Rips finished!";

    return persistence;
}

QVector<double> Persistence::calcHomology(QVector<QVector3D> persistence, double dist)
{
    QVector<double> homo_count(skeleton_,0);
    Q_FOREACH (auto h, persistence) homo_count[h[0]] += h[1] <= dist && dist <= h[2] ? 1 : 0;

    return homo_count;
}
