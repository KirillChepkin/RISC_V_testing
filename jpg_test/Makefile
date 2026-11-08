.PHONY: all test test-all test-color-transform clean
COMPONENTS := jpge_color_transform jpge_forward_dct jpge_huffman_tables jpge_marker_writer

all: test-all

test: test-all

test-color-transform:
	$(MAKE) -C jpge_color_transform test

test-all:
	$(MAKE) -C jpge_color_transform test
	$(MAKE) -C jpge_forward_dct test
	$(MAKE) -C jpge_huffman_tables test
	$(MAKE) -C jpge_marker_writer test

clean:
	@for component in $(COMPONENTS); do $(MAKE) -C $$component clean; done
