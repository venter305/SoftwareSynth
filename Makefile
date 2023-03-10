
define SourcePaths
GraphicsEngine
GraphicsEngine/GUI
GraphicsEngine/GUI/Layout
GraphicsEngine/Input
SoundManager
src
r8brain-free-src
endef

define IncludePaths
/usr/include/freetype2
.
endef

define ExcludeFiles
r8brain-free-src/example.cpp
endef

_CPPFILES=$(foreach dir,$(SourcePaths),$(wildcard $(dir)/*.cpp))

CPPFILES=$(filter-out $(ExcludeFiles),$(_CPPFILES))

_OBJ=$(foreach files,$(CPPFILES),$(patsubst %.cpp,%.o,$(files)))

OBJ=$(patsubst %,./obj/%,$(_OBJ))

INCLUDE=$(foreach dir,$(IncludePaths),-I$(dir))

LFLAGS=-lGLEW -lglfw -lGL -lfreetype -lpulse-simple -lpulse

CXXFLAGS= $(LFLAGS) $(INCLUDE) -O2

synth: $(OBJ)
	g++ $(OBJ) -o synth $(CXXFLAGS)


init:
	mkdir -p obj obj/dep $(foreach dir,$(SourcePaths),obj/$(dir))  $(foreach dir,$(SourcePaths),obj/dep/$(dir))

clean:
	rm -f ./eum ./obj/*.o ./obj/dep/*.d $(foreach dir,$(SourcePaths),obj/$(dir)/*.o) $(foreach dir,$(SourcePaths),obj/dep/$(dir)/*.d)

./obj/dep/%.d: %.cpp
	@rm -f $@
	@g++ -MM $< $(CXXFLAGS) > $@.tmp
	@sed 's,\($(@F:.d=.o)\)[ :]*,$(patsubst obj/dep/%.d,./obj/%.o,$@) :,g' < $@.tmp > $@
	@sed -i '$$a \\tg++ -c $< -o $(patsubst obj/dep/%.d,./obj/%.o,$@) $(CXXFLAGS)' $@
	@rm -f $@.tmp

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),init)
-include $(patsubst %,./obj/dep/%,$(_OBJ:.o=.d))
endif
endif
