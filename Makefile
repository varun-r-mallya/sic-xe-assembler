CXX = clang++
CXXFLAGS = -Wall -Wextra -std=c++23 -O3 -fsanitize=address

BUILDDIR = build

SOURCES = $(wildcard src/*.cc)

OBJECTS = $(SOURCES:%.cc=$(BUILDDIR)/%.o)

TARGET = $(BUILDDIR)/program

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)/src

$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR)/%.o: %.cc | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.d: %.cc | $(BUILDDIR)
	$(CXX) -MM $(CXXFLAGS) $< > $@

run: $(TARGET)
	chmod +x $(TARGET)
	./$(TARGET)
clean:
	rm -rf $(BUILDDIR)

test:
	./$(TARGET) tests/test.sic

.PHONY: all clean
