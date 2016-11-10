//cpp
#include <iostream>
#include <vector>
#include <sstream>

//opencv
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

//larlite
#include "DataFormat/storage_manager.h"
#include "DataFormat/rawdigit.h"
#include "LArUtil/Geometry.h"
#include "LArUtil/DetectorProperties.h"

//prototyes
int interp(float val, float min, float max, float start, float end);
void interp_rgb(float val,int& r,int& g,int& b);
void set_dimension(std::vector<unsigned>& dimension,unsigned int dim,unsigned int plane);

//main
int main(int argc, char** argv) {
  
  if (argc < 3 or argc > 4) {
    std::cout << "\n\tUsage : digit2png [digit file] [event start] [event end]\n\n";
    std::cout << "\t[digit file]    -required-  larlite file containg caldata object\n";
    std::cout << "\t[event start]  -required-  event number to dump. -1 run all\n";
    std::cout << "\t[event end]    -optional-  dump from start to end.\n\n";
    exit(0);
  }

  
  std::string larlite_file=std::string(argv[1]);
  int evt_start = atoi(argv[2]);
  int evt_end = argc == 4 ? atoi(argv[3]) : evt_end =evt_start+1;
  std::cout << argc << " ," << evt_start << " , " << evt_end << "\n";

  larlite::storage_manager storage;
  storage.set_verbosity(larlite::msg::kMSG_TYPE_MAX);
  storage.set_io_mode(larlite::storage_manager::kREAD);
  storage.add_in_filename(larlite_file);
  storage.open();

  if ( evt_start == -1) {
    evt_start=0;
    evt_end  =storage.get_entries();
  }

  storage.go_to(evt_start);

  auto geoservice = larutil::Geometry::GetME();
  auto detprop    = larutil::DetectorProperties::GetME();

  std::vector<float> _plane_data;  
  std::vector<unsigned>  _x_dimensions,_y_dimensions;
  unsigned plane=2;

  set_dimension(_x_dimensions,geoservice->Nwires(plane), plane);
  set_dimension(_y_dimensions,9600, plane);
  _plane_data.resize(_x_dimensions[plane] * _y_dimensions[plane]);
  
  ::cv::Mat mat(_x_dimensions[plane],_y_dimensions[plane],CV_8UC3);
  
  float max=25.0*3;
  float min=0.0;
  
  std::stringstream ss;
  for (int evt=evt_start; evt < evt_end; ++evt) {
    std::cout << "Writing event : " << evt << "\n";
    mat.setTo(cv::Scalar(0, 0, 0)); 
    std::fill(_plane_data.begin(), _plane_data.end(), 0);

    auto ev_digit = (larlite::event_rawdigit*)(storage.get_data<larlite::event_rawdigit>("daq"));

    ss << ev_digit->run() << "_" << ev_digit->subrun() << "_" << ev_digit->event_id() << ".png";

    if(!ev_digit) throw std::exception();

    for (auto const& wire : *ev_digit) {

      unsigned ch_      = wire.Channel();
      unsigned plane_   = geoservice->ChannelToPlane(ch_);
      if (plane_ != plane) continue;
      unsigned detWire_ = geoservice->ChannelToWire(ch_);

      int offset = detWire_ * _y_dimensions[plane_];

      size_t i = 0;
	
      const auto& adcs = wire.ADCs();
      float mean = std::accumulate(adcs.begin(),adcs.end(),0) / (float) adcs.size();
      for(auto adc : wire.ADCs()) {
	{ _plane_data[offset + i] = adc - mean; ++i; }
      }
    }


    int r,g,b;
  
    for (int i = 0; i < mat.rows; ++i) {
      for (int j = 0; j < mat.cols; ++j) {
	float p=_plane_data[i*mat.cols+j];
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
    ::cv::imwrite(ss.str(),mat);
    cv::transpose(mat,mat);
    
    ss.str(std::string());
    ss.clear();

    storage.next_event();
  }
  storage.close();
  
  return 0;
}

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
		   unsigned dim,
		   unsigned plane) {
  
  if (dimension.size() < plane + 1) 
    dimension.resize(plane + 1);
  
  dimension[plane] = dim;
}

