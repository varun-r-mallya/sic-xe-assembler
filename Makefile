CXX = clang++
CXXFLAGS = -Wall -Wextra -std=c++23 -O3 -fsanitize=address

BUILDDIR = build

SOURCES = $(wildcard src/*.cpp)

OBJECTS = $(SOURCES:%.cpp=$(BUILDDIR)/%.o)

TARGET = $(BUILDDIR)/assembler

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)/src

$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR)/%.o: %.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.d: %.cpp | $(BUILDDIR)
	$(CXX) -MM $(CXXFLAGS) $< > $@

run: $(TARGET)
	chmod +x $(TARGET)
	./$(TARGET)
clean:
	rm -rf $(BUILDDIR)

test:
	cp tests/test.sic build/test.sic
	./$(TARGET) -h

.PHONY: all clean
