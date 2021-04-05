# Automatically generated makefile, created by the Projucer
# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!

# build with "V=1" for verbose builds
ifeq ($(V), 1)
V_AT =
else
V_AT = @
endif

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifndef STRIP
  STRIP=strip
endif

ifndef AR
  AR=ar
endif

ifndef CONFIG
  CONFIG=Debug
endif

JUCE_ARCH_LABEL := $(shell uname -m)

ifeq ($(CONFIG),Debug)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Debug
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := 
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) "-DLINUX=1" "-DDEBUG=1" "-D_DEBUG=1" "-DJUCE_DISPLAY_SPLASH_SCREEN=1" "-DJUCE_USE_DARK_SPLASH_SCREEN=1" "-DJUCE_PROJUCER_VERSION=0x60007" "-DJUCE_MODULE_AVAILABLE_juce_core=1" "-DJUCE_MODULE_AVAILABLE_juce_data_structures=1" "-DJUCE_MODULE_AVAILABLE_juce_events=1" "-DJUCE_MODULE_AVAILABLE_juce_graphics=1" "-DJUCE_MODULE_AVAILABLE_juce_gui_basics=1" "-DJUCE_MODULE_AVAILABLE_juce_gui_extra=1" "-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1" "-DJUCE_STRICT_REFCOUNTEDPOINTER=1" "-DJUCE_STANDALONE_APPLICATION=0" "-DJUCE_DLL_BUILD=1" "-DGLFW_INCLUDE_NONE" "-DJUCER_LINUX_MAKE_6D53C8B4=1" "-DJUCE_APP_VERSION=1.0.0" "-DJUCE_APP_VERSION_HEX=0x10000" $(shell pkg-config --cflags freetype2 libcurl webkit2gtk-4.0 gtk+-x11-3.0) -pthread -I../../JuceLibraryCode -I../../External/JUCE/modules -I/usr/local/include -I/usr/include -I../../External/libigl/include -I../../External/libigl/external/eigen -I../../External/libigl/external/glad/include -I../../External/matplotplusplus/source -I../../External/matplotplusplus/source/3rd_party/nodesoup/include -I../../External/matplotplusplus/source/3rd_party/cimg -I../../Source/Public -I../../Source/Private -I../../Source/Private/Objects/Public $(CPPFLAGS)
  JUCE_CPPFLAGS_DYNAMIC_LIBRARY :=  "-DJucePlugin_Build_VST=0" "-DJucePlugin_Build_VST3=0" "-DJucePlugin_Build_AU=0" "-DJucePlugin_Build_AUv3=0" "-DJucePlugin_Build_RTAS=0" "-DJucePlugin_Build_AAX=0" "-DJucePlugin_Build_Standalone=0" "-DJucePlugin_Build_Unity=0"
  JUCE_CFLAGS_DYNAMIC_LIBRARY := -fPIC -fvisibility=hidden
  JUCE_LDFLAGS_DYNAMIC_LIBRARY := -shared
  JUCE_TARGET_DYNAMIC_LIBRARY := libOpenPL.so

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -fPIC -g -ggdb -O0 $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++17 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) -L/usr/local/lib -L/usr/lib $(shell pkg-config --libs freetype2 libcurl) -fvisibility=hidden -lrt -ldl -lpthread -lglfw -lgmp -lmpfr -lboost_thread -lboost_timer -lmatplot -lx11 $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

ifeq ($(CONFIG),Release)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Release
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := 
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) "-DLINUX=1" "-DNDEBUG=1" "-DJUCE_DISPLAY_SPLASH_SCREEN=1" "-DJUCE_USE_DARK_SPLASH_SCREEN=1" "-DJUCE_PROJUCER_VERSION=0x60007" "-DJUCE_MODULE_AVAILABLE_juce_core=1" "-DJUCE_MODULE_AVAILABLE_juce_data_structures=1" "-DJUCE_MODULE_AVAILABLE_juce_events=1" "-DJUCE_MODULE_AVAILABLE_juce_graphics=1" "-DJUCE_MODULE_AVAILABLE_juce_gui_basics=1" "-DJUCE_MODULE_AVAILABLE_juce_gui_extra=1" "-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1" "-DJUCE_STRICT_REFCOUNTEDPOINTER=1" "-DJUCE_STANDALONE_APPLICATION=0" "-DJUCE_DLL_BUILD=1" "-DGLFW_INCLUDE_NONE" "-DJUCER_LINUX_MAKE_6D53C8B4=1" "-DJUCE_APP_VERSION=1.0.0" "-DJUCE_APP_VERSION_HEX=0x10000" $(shell pkg-config --cflags freetype2 libcurl webkit2gtk-4.0 gtk+-x11-3.0) -pthread -I../../JuceLibraryCode -I../../External/JUCE/modules -I/usr/local/include -I/usr/include -I../../External/libigl/include -I../../External/libigl/external/eigen -I../../External/libigl/external/glad/include -I../../External/matplotplusplus/source -I../../External/matplotplusplus/source/3rd_party/nodesoup/include -I../../External/matplotplusplus/source/3rd_party/cimg -I../../Source/Public -I../../Source/Private -I../../Source/Private/Objects/Public $(CPPFLAGS)
  JUCE_CPPFLAGS_DYNAMIC_LIBRARY :=  "-DJucePlugin_Build_VST=0" "-DJucePlugin_Build_VST3=0" "-DJucePlugin_Build_AU=0" "-DJucePlugin_Build_AUv3=0" "-DJucePlugin_Build_RTAS=0" "-DJucePlugin_Build_AAX=0" "-DJucePlugin_Build_Standalone=0" "-DJucePlugin_Build_Unity=0"
  JUCE_CFLAGS_DYNAMIC_LIBRARY := -fPIC -fvisibility=hidden
  JUCE_LDFLAGS_DYNAMIC_LIBRARY := -shared
  JUCE_TARGET_DYNAMIC_LIBRARY := libOpenPL.so

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -fPIC -O3 $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++17 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) -L/usr/local/lib -L/usr/lib $(shell pkg-config --libs freetype2 libcurl) -fvisibility=hidden -lrt -ldl -lpthread -lglfw -lgmp -lmpfr -lboost_thread -lboost_timer -lmatplot -lx11 $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

OBJECTS_DYNAMIC_LIBRARY := \
  $(JUCE_OBJDIR)/algebra_e68384d5.o \
  $(JUCE_OBJDIR)/fruchterman_reingold_9a99f88d.o \
  $(JUCE_OBJDIR)/kamada_kawai_9b0912e.o \
  $(JUCE_OBJDIR)/layout_c7f446e7.o \
  $(JUCE_OBJDIR)/nodesoup_9cb03d36.o \
  $(JUCE_OBJDIR)/bars_3633da06.o \
  $(JUCE_OBJDIR)/box_chart_39ae042e.o \
  $(JUCE_OBJDIR)/circles_285fe0a7.o \
  $(JUCE_OBJDIR)/contours_f0646f1f.o \
  $(JUCE_OBJDIR)/error_bar_4668020.o \
  $(JUCE_OBJDIR)/filled_area_1654414e.o \
  $(JUCE_OBJDIR)/function_line_e8afdbbf.o \
  $(JUCE_OBJDIR)/histogram_b55e0428.o \
  $(JUCE_OBJDIR)/labels_b699b85.o \
  $(JUCE_OBJDIR)/line_e498c93a.o \
  $(JUCE_OBJDIR)/matrix_f8612247.o \
  $(JUCE_OBJDIR)/network_b79f1fb2.o \
  $(JUCE_OBJDIR)/parallel_lines_a9deb54d.o \
  $(JUCE_OBJDIR)/stair_8dd4ab0d.o \
  $(JUCE_OBJDIR)/string_function_9d9982ca.o \
  $(JUCE_OBJDIR)/surface_d7ac5ab1.o \
  $(JUCE_OBJDIR)/vectors_4f068c14.o \
  $(JUCE_OBJDIR)/backend_interface_2d608be4.o \
  $(JUCE_OBJDIR)/backend_registry_1c8a553c.o \
  $(JUCE_OBJDIR)/gnuplot_d13509a5.o \
  $(JUCE_OBJDIR)/axes_object_c073f9c2.o \
  $(JUCE_OBJDIR)/axes_type_3ad9ebbd.o \
  $(JUCE_OBJDIR)/axis_type_8e6cb6c1.o \
  $(JUCE_OBJDIR)/figure_registry_368e79e1.o \
  $(JUCE_OBJDIR)/figure_type_3d9921de.o \
  $(JUCE_OBJDIR)/legend_1cff09ce.o \
  $(JUCE_OBJDIR)/line_spec_903ddbcf.o \
  $(JUCE_OBJDIR)/axes_functions_6cf45b09.o \
  $(JUCE_OBJDIR)/histcounts_bcd111ee.o \
  $(JUCE_OBJDIR)/colors_ff240d6e.o \
  $(JUCE_OBJDIR)/common_f7d3c469.o \
  $(JUCE_OBJDIR)/contourc_3a53ce07.o \
  $(JUCE_OBJDIR)/world_cities_7e5a46d4.o \
  $(JUCE_OBJDIR)/world_map_10m_edfb4f6a.o \
  $(JUCE_OBJDIR)/world_map_50m_c1943c6e.o \
  $(JUCE_OBJDIR)/world_map_110m_a19cbbeb.o \
  $(JUCE_OBJDIR)/glad_e360790d.o \
  $(JUCE_OBJDIR)/OpenPL_CPP_dabd5c4b.o \
  $(JUCE_OBJDIR)/Simulator_eeec9960.o \
  $(JUCE_OBJDIR)/SimulatorBasic_9bbcdbf8.o \
  $(JUCE_OBJDIR)/SimulatorBasic3D_a6aafd29.o \
  $(JUCE_OBJDIR)/SimulatorFDTD_2d5ea08e.o \
  $(JUCE_OBJDIR)/MatPlotPlotter_686b885f.o \
  $(JUCE_OBJDIR)/PL_SCENE_668326e5.o \
  $(JUCE_OBJDIR)/PL_SYSTEM_1515a820.o \
  $(JUCE_OBJDIR)/PLBounds_dc9a924d.o \
  $(JUCE_OBJDIR)/OpenPL_2938867b.o \
  $(JUCE_OBJDIR)/OpenPLCommonPrivate_3e7cb9a7.o \
  $(JUCE_OBJDIR)/include_juce_core_f26d17db.o \
  $(JUCE_OBJDIR)/include_juce_data_structures_7471b1e3.o \
  $(JUCE_OBJDIR)/include_juce_events_fd7d695.o \
  $(JUCE_OBJDIR)/include_juce_graphics_f817e147.o \
  $(JUCE_OBJDIR)/include_juce_gui_basics_e3f79785.o \
  $(JUCE_OBJDIR)/include_juce_gui_extra_6dee1c1a.o \

.PHONY: clean all strip

all : $(JUCE_OUTDIR)/$(JUCE_TARGET_DYNAMIC_LIBRARY)

$(JUCE_OUTDIR)/$(JUCE_TARGET_DYNAMIC_LIBRARY) : $(OBJECTS_DYNAMIC_LIBRARY) $(RESOURCES)
	@command -v pkg-config >/dev/null 2>&1 || { echo >&2 "pkg-config not installed. Please, install it."; exit 1; }
	@pkg-config --print-errors freetype2 libcurl
	@echo Linking "OpenPL - Dynamic Library"
	-$(V_AT)mkdir -p $(JUCE_BINDIR)
	-$(V_AT)mkdir -p $(JUCE_LIBDIR)
	-$(V_AT)mkdir -p $(JUCE_OUTDIR)
	$(V_AT)$(CXX) -o $(JUCE_OUTDIR)/$(JUCE_TARGET_DYNAMIC_LIBRARY) $(OBJECTS_DYNAMIC_LIBRARY) $(JUCE_LDFLAGS) $(JUCE_LDFLAGS_DYNAMIC_LIBRARY) $(RESOURCES) $(TARGET_ARCH)

$(JUCE_OBJDIR)/algebra_e68384d5.o: ../../External/matplotplusplus/source/3rd_party/nodesoup/src/algebra.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling algebra.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/fruchterman_reingold_9a99f88d.o: ../../External/matplotplusplus/source/3rd_party/nodesoup/src/fruchterman_reingold.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling fruchterman_reingold.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/kamada_kawai_9b0912e.o: ../../External/matplotplusplus/source/3rd_party/nodesoup/src/kamada_kawai.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling kamada_kawai.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/layout_c7f446e7.o: ../../External/matplotplusplus/source/3rd_party/nodesoup/src/layout.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling layout.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/nodesoup_9cb03d36.o: ../../External/matplotplusplus/source/3rd_party/nodesoup/src/nodesoup.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling nodesoup.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/bars_3633da06.o: ../../External/matplotplusplus/source/matplot/axes_objects/bars.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling bars.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/box_chart_39ae042e.o: ../../External/matplotplusplus/source/matplot/axes_objects/box_chart.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling box_chart.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/circles_285fe0a7.o: ../../External/matplotplusplus/source/matplot/axes_objects/circles.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling circles.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/contours_f0646f1f.o: ../../External/matplotplusplus/source/matplot/axes_objects/contours.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling contours.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/error_bar_4668020.o: ../../External/matplotplusplus/source/matplot/axes_objects/error_bar.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling error_bar.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/filled_area_1654414e.o: ../../External/matplotplusplus/source/matplot/axes_objects/filled_area.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling filled_area.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/function_line_e8afdbbf.o: ../../External/matplotplusplus/source/matplot/axes_objects/function_line.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling function_line.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/histogram_b55e0428.o: ../../External/matplotplusplus/source/matplot/axes_objects/histogram.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling histogram.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/labels_b699b85.o: ../../External/matplotplusplus/source/matplot/axes_objects/labels.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling labels.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/line_e498c93a.o: ../../External/matplotplusplus/source/matplot/axes_objects/line.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling line.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/matrix_f8612247.o: ../../External/matplotplusplus/source/matplot/axes_objects/matrix.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling matrix.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/network_b79f1fb2.o: ../../External/matplotplusplus/source/matplot/axes_objects/network.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling network.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/parallel_lines_a9deb54d.o: ../../External/matplotplusplus/source/matplot/axes_objects/parallel_lines.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling parallel_lines.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/stair_8dd4ab0d.o: ../../External/matplotplusplus/source/matplot/axes_objects/stair.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling stair.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/string_function_9d9982ca.o: ../../External/matplotplusplus/source/matplot/axes_objects/string_function.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling string_function.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/surface_d7ac5ab1.o: ../../External/matplotplusplus/source/matplot/axes_objects/surface.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling surface.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/vectors_4f068c14.o: ../../External/matplotplusplus/source/matplot/axes_objects/vectors.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling vectors.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/backend_interface_2d608be4.o: ../../External/matplotplusplus/source/matplot/backend/backend_interface.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling backend_interface.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/backend_registry_1c8a553c.o: ../../External/matplotplusplus/source/matplot/backend/backend_registry.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling backend_registry.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/gnuplot_d13509a5.o: ../../External/matplotplusplus/source/matplot/backend/gnuplot.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling gnuplot.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/axes_object_c073f9c2.o: ../../External/matplotplusplus/source/matplot/core/axes_object.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling axes_object.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/axes_type_3ad9ebbd.o: ../../External/matplotplusplus/source/matplot/core/axes_type.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling axes_type.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/axis_type_8e6cb6c1.o: ../../External/matplotplusplus/source/matplot/core/axis_type.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling axis_type.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/figure_registry_368e79e1.o: ../../External/matplotplusplus/source/matplot/core/figure_registry.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling figure_registry.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/figure_type_3d9921de.o: ../../External/matplotplusplus/source/matplot/core/figure_type.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling figure_type.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/legend_1cff09ce.o: ../../External/matplotplusplus/source/matplot/core/legend.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling legend.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/line_spec_903ddbcf.o: ../../External/matplotplusplus/source/matplot/core/line_spec.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling line_spec.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/axes_functions_6cf45b09.o: ../../External/matplotplusplus/source/matplot/freestanding/axes_functions.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling axes_functions.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/histcounts_bcd111ee.o: ../../External/matplotplusplus/source/matplot/freestanding/histcounts.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling histcounts.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/colors_ff240d6e.o: ../../External/matplotplusplus/source/matplot/util/colors.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling colors.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/common_f7d3c469.o: ../../External/matplotplusplus/source/matplot/util/common.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling common.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/contourc_3a53ce07.o: ../../External/matplotplusplus/source/matplot/util/contourc.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling contourc.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/world_cities_7e5a46d4.o: ../../External/matplotplusplus/source/matplot/util/world_cities.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling world_cities.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/world_map_10m_edfb4f6a.o: ../../External/matplotplusplus/source/matplot/util/world_map_10m.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling world_map_10m.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/world_map_50m_c1943c6e.o: ../../External/matplotplusplus/source/matplot/util/world_map_50m.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling world_map_50m.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/world_map_110m_a19cbbeb.o: ../../External/matplotplusplus/source/matplot/util/world_map_110m.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling world_map_110m.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/glad_e360790d.o: ../../External/libigl/external/glad/src/glad.c
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling glad.c"
	$(V_AT)$(CC) $(JUCE_CFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/OpenPL_CPP_dabd5c4b.o: ../../Source/Private/CPP/OpenPL_CPP.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling OpenPL_CPP.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/Simulator_eeec9960.o: ../../Source/Private/Objects/Private/Simulators/Simulator.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling Simulator.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/SimulatorBasic_9bbcdbf8.o: ../../Source/Private/Objects/Private/Simulators/SimulatorBasic.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling SimulatorBasic.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/SimulatorBasic3D_a6aafd29.o: ../../Source/Private/Objects/Private/Simulators/SimulatorBasic3D.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling SimulatorBasic3D.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/SimulatorFDTD_2d5ea08e.o: ../../Source/Private/Objects/Private/Simulators/SimulatorFDTD.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling SimulatorFDTD.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/MatPlotPlotter_686b885f.o: ../../Source/Private/Objects/Public/MatPlotPlotter.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling MatPlotPlotter.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/PL_SCENE_668326e5.o: ../../Source/Private/Objects/Private/PL_SCENE.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling PL_SCENE.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/PL_SYSTEM_1515a820.o: ../../Source/Private/Objects/Private/PL_SYSTEM.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling PL_SYSTEM.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/PLBounds_dc9a924d.o: ../../Source/Private/Objects/Private/PLBounds.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling PLBounds.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/OpenPL_2938867b.o: ../../Source/Private/OpenPL.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling OpenPL.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/OpenPLCommonPrivate_3e7cb9a7.o: ../../Source/Private/OpenPLCommonPrivate.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling OpenPLCommonPrivate.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_core_f26d17db.o: ../../JuceLibraryCode/include_juce_core.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_core.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_data_structures_7471b1e3.o: ../../JuceLibraryCode/include_juce_data_structures.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_data_structures.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_events_fd7d695.o: ../../JuceLibraryCode/include_juce_events.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_events.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_graphics_f817e147.o: ../../JuceLibraryCode/include_juce_graphics.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_graphics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_gui_basics_e3f79785.o: ../../JuceLibraryCode/include_juce_gui_basics.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_gui_basics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_gui_extra_6dee1c1a.o: ../../JuceLibraryCode/include_juce_gui_extra.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_gui_extra.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_DYNAMIC_LIBRARY) $(JUCE_CFLAGS_DYNAMIC_LIBRARY) -o "$@" -c "$<"

clean:
	@echo Cleaning OpenPL
	$(V_AT)$(CLEANCMD)

strip:
	@echo Stripping OpenPL
	-$(V_AT)$(STRIP) --strip-unneeded $(JUCE_OUTDIR)/$(TARGET)

-include $(OBJECTS_DYNAMIC_LIBRARY:%.o=%.d)
