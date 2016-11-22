#include <igl/avg_edge_length.h>
#include <igl/cotmatrix.h>
#include <igl/invert_diag.h>
#include <igl/massmatrix.h>
#include <igl/parula.h>
#include <igl/per_corner_normals.h>
#include <igl/per_face_normals.h>
#include <igl/per_vertex_normals.h>
#include <igl/principal_curvature.h>
#include <igl/readOBJ.h>
#include <igl/readPLY.h>
#include <igl/read_triangle_mesh.h>
#include <igl/viewer/Viewer.h>

#include <fstream>
#include <iostream>
#include <string>

void usage(const std::string &basename);

Eigen::MatrixXd V;
Eigen::MatrixXd UV;
Eigen::MatrixXd N;
Eigen::MatrixXi F;
Eigen::MatrixXi Fuv, Fn;

void writeCurvature( const Eigen::VectorXd &curv, const std::string &filename);

int main(int argc, char *argv[])
{
  using namespace Eigen;

  std::string filename;
  if(argc > 1) {
    filename = argv[1];
  } else {
    usage(argv[0]);
    std::cerr << "Required mesh filename missing!\n";
    return 1;
  }

  enum CurvatureType {
    None  = 0,
    Mean  = 1,
    Gauss = 2,
    K1    = 3,
    K2    = 4,
    K     = 5,
    All   = 6
  } curvType = None;
  if(argc > 2) {
    std::string curvStr = argv[2];
    if(curvStr == "mean")       curvType = Mean;
    else if(curvStr == "gauss") curvType = Gauss;
    else if(curvStr == "k1")    curvType = K1;
    else if(curvStr == "k2")    curvType = K2;
    else if(curvStr == "k")     curvType = K;
    else if(curvStr == "all")   curvType = All;
    else {
      usage(argv[0]);
      std::cerr << "Invalid curvature type: " << curvStr << "\n";
      return 1;
    }
  }

  bool show = true;
  if(argc > 3) {
    show = argv[3] == "1";
  }

  bool writeData = false;
  // Load a mesh, can only output data for OBJ / PLY with UV data
  std::string fileExt = filename.substr(filename.size() - 4);
  if(fileExt == ".obj" || fileExt == ".OBJ") {
    writeData = igl::readOBJ(filename, V, UV, N, F, Fuv, Fn);
  } else if(fileExt == ".ply" || fileExt == ".PLY") {
    writeData = igl::readPLY(filename, V, F, N, UV);
  }
  if(!writeData){
    igl::read_triangle_mesh(filename, V, F);
  }

  // Information
  std::cout << "V: " << V.rows() << "," << V.cols() << "\n";
  std::cout << "F: " << F.rows() << "," << F.cols() << "\n";
  std::cout << "N: " << N.rows() << "," << N.cols() << "\n";
  std::cout << "UV: " << UV.rows() << "," << UV.cols() << "\n";
  std::cout << "Fuv: " << Fuv.rows() << "," << Fuv.cols() << "\n";
  std::cout << "Fn: " << Fn.rows() << "," << Fn.cols() << "\n";

  // Alternative discrete mean curvature
  MatrixXd HN;
  SparseMatrix<double> L,M,Minv;
  igl::cotmatrix(V,F,L);
  igl::massmatrix(V,F,igl::MASSMATRIX_TYPE_VORONOI,M);
  igl::invert_diag(M,Minv);
  // Laplace-Beltrami of position
  HN = -Minv*(L*V);
  // Extract magnitude as mean curvature
  VectorXd H = HN.rowwise().norm();

  // Compute curvature directions via quadric fitting
  MatrixXd PD1,PD2;
  VectorXd PV1,PV2;
  igl::principal_curvature(V,F,PD1,PD2,PV1,PV2);
  // mean curvature
  H = 0.5*(PV1+PV2);
  // gaussian curvature
  VectorXd G = PV1 * PV2;


  if(show){
    igl::viewer::Viewer viewer;
    viewer.data.set_mesh(V, F);
  
    // Compute pseudocolor
    MatrixXd C;
    VectorXd *curvature = &H;
    switch(curvType){
      case K1: curvature = &PV1; break;
      case K2: curvature = &PV2; break;
      case Gauss: curvature = &G; break;
      default:
      case Mean: curvature = &H; break;
    }
    igl::parula(*curvature, true, C);
    viewer.data.set_colors(C);
  
    // Average edge length for sizing
    const double avg = igl::avg_edge_length(V,F);
  
    // Draw a blue segment parallel to the minimal curvature direction
    const RowVector3d red(0.8,0.2,0.2),blue(0.2,0.2,0.8);
    viewer.data.add_edges(V + PD1*avg, V - PD1*avg, blue);
  
    // Draw a red segment parallel to the maximal curvature direction
    viewer.data.add_edges(V + PD2*avg, V - PD2*avg, red);
  
    // Hide wireframe
    viewer.core.show_lines = false;
  
    viewer.launch();
  } else if(!writeData){
    std::cerr << "No UV available in " << filename << "!\n";
    return 1;
  }

  if(writeData){
    switch(curvType){
      case Mean:
        writeCurvature(H, filename + "-H.tsv");
        break;
      case Gauss:
        writeCurvature(G, filename + "-G.tsv");
        break;
      case K1:
        writeCurvature(PV1, filename + "-K1.tsv");
        break;
      case K2:
        writeCurvature(PV2, filename + "-K2.tsv");
        break;
      case K:
        writeCurvature(PV1, filename + "-K1.tsv");
        writeCurvature(PV2, filename + "-K2.tsv");
        break;
      case All:
        writeCurvature(H, filename + "-H.tsv");
        writeCurvature(G, filename + "-G.tsv");
        writeCurvature(PV1, filename + "-K1.tsv");
        writeCurvature(PV2, filename + "-K2.tsv");
        break;
      default:
        break;
    }
  }
}

void usage(const std::string &basename) {
  std::cout << "Usage: " << basename << " mesh [mean | gauss | k1 | k2 | k | all] [show]\n";
  std::cout << "   mean:   mean curvature\n";
  std::cout << "   gauss:  gaussian curvature\n";
  std::cout << "   k1:     first main curvature component\n";
  std::cout << "   k2:     second main curvature component\n";
  std::cout << "   k:      both main curvature components\n";
  std::cout << "   all:    all variants\n";
  std::cout << "\n";
  std::cout << "   show: whether to display the mesh curvature (default, 1) or not (0)\n";
  std::cout << "\n";
}

void writeCurvature( const Eigen::VectorXd &curv, const std::string &filename) {
  
  enum {
    VertexUV = 0,
    FaceUV   = 1,
    Invalid  = 2
  } mode = Invalid;
  if(curv.rows() == UV.rows())
    mode = VertexUV;
  else if(curv.rows() == V.rows() && Fuv.rows() == F.rows())
    mode = FaceUV;

  if(mode == Invalid){
    std::cerr << "Unsupported UV data:\n";
    std::cerr << "UV=" << UV.rows() << "," << UV.cols() << "\n";
    std::cerr << "Fuv=" << Fuv.rows() << "," << Fuv.cols() << "\n";
    std::cerr << "Curv=" << curv.rows() << " from curvature\n";
    return;
  }
  
  std::ofstream out(filename);
  if(!out.is_open()){
    std::cerr << "Cannot write to " << filename << "\n";
    return;
  }

  if(mode == VertexUV){
    // UV per vertex
    for(int i = 0; i < curv.rows(); ++i){
      if(i > 0) out << "\n";
      out << UV(i, 0) << "\t" << UV(i, 1) << "\t" << curv(i);
    }
  } else {
    // UV per vertex per face
    for(int f = 0; f < Fuv.rows(); ++f){
      for(int i = 0; i < Fuv.cols(); ++i){
        if(f > 0 || i > 0) out << "\n";
        int uv_idx = Fuv(f, i);
        int v_idx  = F(f, i);
        out << UV(uv_idx, 0) << "\t" << UV(uv_idx, 1) << "\t" << curv(v_idx);
      }
    }
  }

  std::cout << "Saved curvature to " << filename << "\n";
}

