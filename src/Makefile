BOOST_INCLUDE_DIR=/mnt/hgfs/mDAX/trunk/Common/include/boost
BOOST_LIB_DIR=/mnt/hgfs/mDAX/trunk/common/lib/boost/linux
TestEasyLog: TestEasyLog.h TestEasyLog.cpp EasyLog.h EasyLog.cpp
	g++ $^ -o $@ -lboost_system -lboost_thread  -I$(BOOST_INCLUDE_DIR) -L$(BOOST_LIB_DIR) -lpthread
