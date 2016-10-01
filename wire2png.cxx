#include <iostream>


#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>


#include "DataFormat/storage_manager.h"
#include "DataFormat/wire.h"


#include "LArUtil/Geometry.h"
#include "LArUtil/DetectorProperties.h"


int interp(float val, float min, float max, float start, float end) {
  return ((val-min)/(max-min))*(end-start)+start;
}

void interp_rgb(float val,int& r,int& g,int& b) {

  float v1,v2;

  v1=0;
  v2=0.33333;
  
  if (v1<=val and val<=v2) {
    r = interp(val,v1,v2,22,0); 
    g = interp(val,v1,v2,30,181);
    b = interp(val,v1,v2,151,226);
    return;
  }

  v1=0.33333;
  v2=0.47;
  if (v1<val and val<=v2) {
    r=interp(val,v1,v2,0,76);
    g=interp(val,v1,v2,181,140);
    b=interp(val,v1,v2,226,43);
    return;
  }
  
  v1=0.47;
  v2=0.645;
  if (v1<val and val<=v2) {
    r= interp(val,v1,v2,76,0);
    g=interp(val,v1,v2,140,206);
    b=interp(val,v1,v2,43,24);
    return;
  }

  v1=0.645;
  v2=0.791;
  if (v1<val and val<=v2) {
    r=interp(val,v1,v2,0,254);
    g=interp(val,v1,v2,206,209);
    b=interp(val,v1,v2,24,65);
    return;
  }
    

  v1=0.791;
  v2=1.0;
  if (v1<val and val<=v2) {
    r = interp(val,v1,v2,254,255);
    g = interp(val,v1,v2,209,0);
    b = interp(val,v1,v2,65,0);
    return;
  }

  
}
void set_dimension(std::vector<unsigned>& dimension,
		   unsigned int dim,
		   unsigned int plane) {
  
  if (dimension.size() < plane + 1) 
    dimension.resize(plane + 1);
  
  dimension[plane] = dim;
}


void init_data_holder(std::vector<std::vector<float> >& data,
		      const std::vector<unsigned>& x_dimensions,
		      const std::vector<unsigned>& y_dimensions) {
  data.resize(x_dimensions.size());

  for (size_t i = 0; i < x_dimensions.size(); i ++ ) 
    data[i].resize(x_dimensions[i] * y_dimensions[i]);
}


int main(int arc, char** argv) {
  larlite::storage_manager storage;
  storage.set_verbosity(larlite::msg::kMSG_TYPE_MAX);
  storage.set_io_mode(larlite::storage_manager::kREAD);
  storage.add_in_filename("/Users/vgenty/Desktop/pi0_files/selected/larlite_wire_filtered.root");
  storage.open();
  storage.go_to(0);
  auto ev_wire = (larlite::event_wire*)(storage.get_data<larlite::event_wire>("caldata"));
  if(!ev_wire) throw std::exception();
  auto geoservice = larutil::Geometry::GetME();
  auto detprop = larutil::DetectorProperties::GetME();

  std::vector<unsigned> _x_dimensions,_y_dimensions;//why do we need this
  
  for (unsigned int p = 0; p < geoservice -> Nviews(); p ++) {
    set_dimension(_x_dimensions,geoservice->Nwires(p), p);
    set_dimension(_y_dimensions,detprop->ReadOutWindowSize(), p);
  }
  std::vector<std::vector<float > > _plane_data; // data to png
  init_data_holder(_plane_data,_x_dimensions,_y_dimensions);
  
  for (auto const& wire : *ev_wire) {
    unsigned int ch_ = wire.Channel();
    unsigned int detWire_ = geoservice->ChannelToWire(ch_);
    unsigned int plane_ = geoservice->ChannelToPlane(ch_);
    int offset = detWire_ * _y_dimensions[plane_];

    for (const auto & roi : wire.SignalROI().get_ranges()) {
      const int start_tick = roi.begin_index();
      size_t i = 0;
      for (float adc : roi)
        { _plane_data[plane_][offset + start_tick + i] = adc; ++i; }
    }
  }

  ::cv::Mat mat(_x_dimensions[2],_y_dimensions[2],CV_8UC3);

  float max=25.0;
  float min=0.0;
  int r,g,b;
  for (int i = 0; i < mat.rows; ++i) {
    for (int j = 0; j < mat.cols; ++j) {
      float p=_plane_data[2][i*mat.cols+j];
      if (p>max) p = max;
      if (p<min) p = min;
      p/=max;
      auto &intensity = mat.at<cv::Vec3b>(i, j);
      interp_rgb(p,r,g,b);
      intensity.val[0] = b;
      intensity.val[1] = g;
      intensity.val[2] = r;
    }
  }

  cv::transpose(mat,mat);
  ::cv::imwrite("aho.png",mat);
		
  storage.close();
  
  return 0;
}
