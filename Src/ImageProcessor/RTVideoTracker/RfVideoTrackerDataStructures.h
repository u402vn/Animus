#pragma once
#include <stdint.h>
#include <stddef.h>

namespace rf {

#define RF_VIDEO_TRACKER_VERSION_MAJOR 3
#define RF_VIDEO_TRACKER_VERSION_MINOR 1

	/// Tracker constants
#define RF_MAX_STROBE_W 128		/// Maximum tracking rectangle width
#define RF_MAX_STROBE_H 128		/// Maximum tracking rectangle height
#define RF_MIN_STROBE_W 16		/// Minimum tracking rectangle width					
#define RF_MIN_STROBE_H 16		/// Minimum tracking rectangle height
#define RF_MIN_FRAME_W 240		/// Minimum image width
#define RF_MIN_FRAME_H 240		/// Minimum image height
#define RF_MAX_CORR_W 105		/// Maximum correlation surface width. RF_MAX_WIND_W = RF_MAX_CORR_W + RF_MAX_STROBE_W - 1
#define RF_MAX_CORR_H 105		/// Maximum correlation surface height. RF_MAX_WIND_H = RF_MAX_CORR_H + RF_MAX_STROBE_H - 1
#define RF_MIN_CORR_W 27		/// Minimum correlation surface width. RF_MAX_WIND_W = RF_MAX_CORR_W + RF_MAX_STROBE_W - 1
#define RF_MIN_CORR_H 27		/// Minimum correlation surface height. RF_MAX_WIND_H = RF_MAX_CORR_H + RF_MAX_STROBE_H - 1			
#define RF_MAX_NUM_THREADS 16	/// Maximum number of computation threads
#define RF_MAX_NUM_LOCAL_MAX 4	/// Maximum number of local maximum

/// Default tracker params
#define	RF_DEFAULT_FRAME_WIDTH 0					/// Width of images for processing
#define	RF_DEFAULT_FRAME_HEIGHT 0					/// Height of images for processing
#define	RF_DEFAULT_FRAME_BUFF_SIZE 2				/// Size of frame buffer for timelapse function
#define	RF_DEFAULT_STROBE_W 128						/// Width of tracking rectangle
#define	RF_DEFAULT_STROBE_H 128						/// Height of tracking rectangle
#define	RF_DEFAULT_PIXEL_DEVIATION_THRESHOLD 0		/// Pixels value deviations for object presence check
#define	RF_DEFAULT_NUM_THREADS 0					/// Number of computational threads
#define	RF_DEFAULT_OBJECT_LOSS_THRESHOLD 0.3f		/// Threshold for loss tracking detection
#define	RF_DEFAULT_OBJECT_DETECTION_THRESHOLD 0.3f	/// Threshold for object detection after loss
#define	RF_DEFAULT_SEARCH_WINDOW_OFFSET_X 0			/// Horithontal offset of search window
#define	RF_DEFAULT_SEARCH_WINDOW_OFFSET_Y 0			/// Vertiacal offset of search window
#define	RF_DEFAULT_PERMANENT_SEARCH_WINDOW_OFFSET 0	/// Permanent search windows offset flag
#define	RF_DEFAULT_PATTERN_UPDATE_COEFF 0.95f		/// Coefficient for updating of pattern image
#define	RF_DEFAULT_PROBABILITY_COEFF 0.95f			/// Coefficient for updating of probability threshold
#define	RF_DEFAULT_VELOCITY_COEFF 0.95f				/// Coefficient for updating of velocity
#define	RF_DEFAULT_NUM_LOCAL_MAXIMUMS 2				/// Number of alternative object position for check
#define	RF_DEFAULT_CORR_WIDTH 105					/// Width of correlation threshold (number of position for object check)
#define	RF_DEFAULT_CORR_HEIGHT 105					/// Height of correlation threshold (number of position for object check)
#define	RF_DEFAULT_LOST_MODE_OPTION 0				/// LOST mode option (0, 1 or 2)
#define RF_DEFAULT_MAX_NUM_FRAMES_IN_LOST_MODE 256	/// Maximum number of frames in LOST mode for autoreset

/// Tracker modes indexes
#define RF_FREE_MODE_INDEX 0		/// FREE mode index
#define RF_TRACKING_MODE_INDEX 1	/// TRACKING mode index
#define RF_LOST_MODE_INDEX 2		/// LOST mode index
#define RF_INERTIAL_MODE_INDEX 3	/// INERTIAL mode index
#define RF_STATIC_MODE_INDEX 4		/// STATIC mode index

	/// Tracker commands enum
	enum class RfVideoTrackerCommand {
		CAPTURE = 1,				/// Capture object command
		RESET = 2,					/// Reset command
		SET_INERTIAL_MODE = 3,		/// Set inertial mode command
		SET_LOST_MODE = 4,			/// Set lost mode command
		SET_STATIC_MODE = 5,		/// Set static mode command
		SET_STROBE_AUTO_SIZE = 6,	/// Set strobe size automaticaly command
		MOVE_STROBE = 7,			/// Move tracking rectangle command
		PROCESS_FRAME = 8,			/// Process frame
		MOVE_STROBE_AUTO = 9		/// Set strobe to auto position
	};

	/// tracker property enum
	enum class RfVideoTrackerProperty {
		FRAME_WIDTH = 1,					/// Width of images for processing
		FRAME_HEIGHT = 2,					/// Height of images for processing
		FRAME_BUFF_SIZE = 3,				/// Size of frame buffer for timelapse function
		STROBE_WIDTH = 4,					/// Width of tracking rectangle
		STROBE_HEIGHT = 5,					/// Height of tracking rectangle
		PIXEL_DEVIATION_THRESHOLD = 8,		/// Pixels value deviations for object presence check
		NUM_THREADS = 9,					/// Number of computational threads
		OBJECT_LOSS_THRESHOLD = 10,			/// Threshold for loss tracking detection
		OBJECT_DETECTION_THRESHOLD = 11,	/// Threshold for object detection after loss
		SEARCH_WINDOW_OFFSET_X = 12,		/// Horithontal offset of search window
		SEARCH_WINDOW_OFFSET_Y = 13,		/// Vertiacal offset of search window
		PERMANENT_SEARCH_WINDOW_OFFSET = 14,/// Permanent search windows offset flag
		PATTERN_UPDATE_COEFF = 15,			/// Coefficient for updating of pattern image
		PROBABILITY_COEFF = 16,				/// Coefficient for updating of probability threshold
		VELOCITY_COEFF = 17,				/// Coefficient for updating of velocity
		NUM_LOCAL_MAXIMUMS = 18,			/// Number of alternative object position for check
		CORR_WIDTH = 19,					/// Width of correlation threshold (number of position for object check)
		CORR_HEIGHT = 20,					/// Height of correlation threshold (number of position for object check)
		LOST_MODE_OPTION = 21,				/// LOST mode option (0, 1 or 2)
		MAX_NUM_FRAMES_IN_LOST_MODE = 22,	/// Maximum number of frames in LOST mode for autoreset
		ALGORITHM_TYPE = 23					/// Tracking algorithm type
	};

	/// tracker image type enum
	enum class RfImageType {
		PATTERN_IMAGE = 1,	/// Pattern image
		MASK_IMAGE = 2,		/// Mask image
		CORR_IMAGE = 3		/// Correlation surface image
	};

	/// Tracker data structure
	typedef struct {
		int32_t strobe_x;							/// Integer horizontal position of tracking rectangle center
		int32_t strobe_y;							/// Integer vertical position of tracking rectangle center
		int32_t strobe_w;							/// Tracking rectangle width
		int32_t strobe_h;							/// Tracking rectangle height
		int32_t substrobe_x;						/// Horizontal position of object rectangle in tracking rectangle
		int32_t substrobe_y;						/// Vertical position of object rectangle in tracking rectangle
		int32_t substrobe_w;						/// Object rectangle width in racking rectangle
		int32_t substrobe_h;						/// Object rectangle height in tracking rectangle
		int32_t search_wind_dx;						/// Horizontal offset of search window 
		int32_t search_wind_dy;						/// Vertical offset of search window
		int32_t lost_frame_count;					/// Number of frames in LOST mode
		int32_t frame_count;						/// Number of frames in TRACKING, LOST, INERTIAL and STATIC modes
		int32_t frame_w;							/// Width of processed frame
		int32_t frame_h;							/// Height og processed frame
		int32_t corr_w;								/// Correlation surface width
		int32_t corr_h;								/// Correlation surface height
		int32_t pixel_deviation_threshold;			/// Threshold of pixel value deviation for calculation of object presense
		int32_t num_threads;						/// Number of computational threads
		int32_t lost_mode_option;					/// LOST mode option
		int32_t frame_buf_size;						/// Size of frame buffer
		int32_t num_local_max;						/// Num local maximums for check object presence
		int32_t max_num_frames_in_lost_mode;		/// Maximum num frames in LOST mode
		int32_t tracker_frame_id;					/// ID of last processed frame
		int32_t buf_frame_id;						/// ID of last added frame in buf
		int32_t algorithm_type;						/// Type of tracking algorithm
		float f_strobe_x;							/// Subpixel position of tracking rectangle center
		float f_strobe_y;							/// Subpixel position of tracking rectangle center
		float vel_x;								/// Horizontal speed of object on frames
		float vel_y;								/// Vertical speed of object on frames
		float object_loss_threshold;				/// Threshold for loss tracking detection
		float object_detection_threshold;			/// Threshold for object detection
		float pattern_update_coeff;					/// Coefficient of pattern updating
		float velocity_update_coeff;				/// Coeff of velocity updating
		float probability_threshold_coeff;			/// Coefficient for updating probability threshold
		float corr_p;								/// Probability of object detection
		uint8_t mode;								/// Mode ID
		uint8_t permanent_search_wind_offset_flag;	/// 1 - permanent serach window offset, 0 - not peramnent search window offset
	} RfVideoTrackerData;

}//namespace...
