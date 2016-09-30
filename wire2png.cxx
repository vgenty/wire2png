#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>


#include "DataFormat/storage_manager.h"
#include "DataFormat/wire.h"


#include "LArUtil/Geometry.h"
#include "LArUtil/DetectorProperties.h"



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
  std::cout << "start\n";
  larlite::storage_manager storage{};
  storage.set_io_mode(larlite::storage_manager::kREAD);
  storage.add_in_filename("/Users/vgenty/Desktop/pi0_files/selected/larlite_wire_filtered.root");
  storage.open();
  storage.go_to(0);
  auto ev_wire = (larlite::event_wire*)(storage.get_data<larlite::event_wire>("caldata"));
  if(!ev_wire) throw std::exception();
  std::cout << "Got event wire pointer : " << ev_wire << "\n";
  std::cout << "Getting geometry service...\n";
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

    for (const auto & iROI : wire.SignalROI().get_ranges()) {
      const int FirstTick = iROI.begin_index();
      size_t i = 0;
      for (float ADC : iROI)
        { _plane_data[plane_][offset + FirstTick + i] = ADC; ++i; }
      }
    }
  

  for(short i=0;i<3;++i)
    std::cout << i << "," << _plane_data[i].size() << "\n";;
  std::cout << _x_dimensions.size() << "," << _y_dimensions.size() << "\n";
  ::cv::Mat mat((_x_dimensions[2],_y_dimensions[2]));
  
  storage.close();
  std::cout << "end\n";  
  
  //   ::larlite::event_PiZeroROI* ev_roi = nullptr;
  //   if ( _use_roi ) {

  //     ev_roi = storage->get_data<event_PiZeroROI>( "mcroi" );
  //     if(ev_roi->size() == 0) throw DataFormatException("Could not locate ROI data product And you have UseROI: True!");

  //     auto wr_v = (*ev_roi)[0].GetWireROI();
  //     auto tr_v = (*ev_roi)[0].GetTimeROI();
      
  //     for(uint k=0; k < nplanes; ++k)
  // 	{ wire_range_v[k] = wr_v[k]; tick_range_v[k] = tr_v[k]; }

  //   } else {
    
  //     for(auto const& wire_data : *ev_wire) {
      
  // 	auto const& wid = geom->ChannelToWireID(wire_data.Channel());
      
  // 	auto& wire_range = wire_range_v[wid.Plane];
  // 	if(wire_range.first  > wid.Wire) wire_range.first  = wid.Wire;
  // 	if(wire_range.second < wid.Wire) wire_range.second = wid.Wire;
      
  // 	auto& tick_range = tick_range_v[wid.Plane];
      
  // 	auto const& roi_v = wire_data.SignalROI();
      
  // 	for(auto const& roi : roi_v.get_ranges()) {	
  // 	  size_t start_tick = roi.begin_index();
  // 	  size_t last_tick = start_tick + roi.size() - 1;
  // 	  if(tick_range.first  > start_tick) tick_range.first  = start_tick;
  // 	  if(tick_range.second < last_tick)  tick_range.second = last_tick;
  // 	}
  //     }
  //   }
    
  //   for(size_t plane=0; plane<nplanes; ++plane) {
  //     auto const& wire_range = wire_range_v[plane];
  //     auto const& tick_range = tick_range_v[plane];
  //     size_t nticks = tick_range.second - tick_range.first + 2;
  //     size_t nwires = wire_range.second - wire_range.first + 2;


  //     ::larocv::ImageMeta meta((double)nwires,(double)nticks,nwires,nticks,wire_range.first,tick_range.first,plane);
  //     if ( _use_roi ) {
  // 	const auto& vtx = (*ev_roi)[0].GetVertex()[plane];
  // 	meta.setvtx(vtx.first,vtx.second);
  //     }
      
  //     if ( nwires >= 1e10 || nticks >= 1e10 )
  // 	_img_mgr.push_back(cv::Mat(),::larocv::ImageMeta());
  //     else
  // 	_img_mgr.push_back(::cv::Mat(nwires, nticks, CV_8UC1, cvScalar(0.)),meta);
  //   }

  //   for(auto const& wire_data : *ev_wire) {
      
  //     auto const& wid = geom->ChannelToWireID(wire_data.Channel());

  //     auto& mat = _img_mgr.img_at(wid.Plane);
      
  //     auto const& roi_v = wire_data.SignalROI();

  //     auto const& wire_range = wire_range_v[wid.Plane];
  //     auto const& tick_range = tick_range_v[wid.Plane];

  //     for(auto const& roi : roi_v.get_ranges()) {
  // 	size_t start_tick = roi.begin_index();
  // 	for(size_t adc_index=0; adc_index < roi.size(); ++adc_index) {
  // 	  size_t x = wid.Wire - wire_range.first;
  // 	  size_t y = start_tick + adc_index - tick_range.first;
  // 	  double q = roi[adc_index] / _charge_to_gray_scale;
  // 	  q += (double)(mat.at<unsigned char>(x,y));
  // 	  if( q <  0) q = 0.;
  // 	  if( q >255) q = 255;
  // 	  mat.at<unsigned char>(x,y) = (unsigned char)((int)q);
  // 	}
  //     }
  //   }

  //   if ( _pool_time_tick > 1 ) {

  //     for(size_t plane=0; plane<nplanes; ++plane) {

  // 	auto& img  = _img_mgr.img_at(plane);
  // 	auto& meta = _img_mgr.meta_at(plane);

  // 	::cv::Mat pooled(img.rows, img.cols/_pool_time_tick+1, CV_8UC1, cvScalar(0.));
      
  // 	for(int row = 0; row < img.rows; ++row) {

  // 	  uchar* p = img.ptr(row);
	  
  // 	  for(int col = 0; col < img.cols; ++col) {

  // 	    int pp = *p++;
	  
  // 	    auto& ch = pooled.at<uchar>(row,col/_pool_time_tick);

  // 	    int res  = pp + (int) ch;
  // 	    if (res > 255) res = 255;
  // 	    ch = (uchar) res;
	  
  // 	  }
  // 	}

  // 	//old parameters
  // 	auto const& wire_range = wire_range_v[plane];
  // 	auto const& tick_range = tick_range_v[plane];

  // 	img  = pooled;
  // 	meta = ::larocv::ImageMeta((double)pooled.rows,
  // 				  (double)pooled.cols*_pool_time_tick,
  // 				  pooled.rows,
  // 				  pooled.cols,
  // 				  wire_range.first,
  // 				  tick_range.first,
  // 				  plane);
  // 	if ( _use_roi ) {
  // 	  const auto& vtx = (*ev_roi)[0].GetVertex()[plane];
  // 	  meta.setvtx(vtx.first,vtx.second);
  // 	}

  //     }
  //   }
  return 0;
}
