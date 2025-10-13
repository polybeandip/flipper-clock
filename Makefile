.PHONY: upload compile

PORT ?= $(error Port undefined)

upload: compile
	arduino-cli upload -p $(PORT) --fqbn esp32:esp32:esp32 .

compile:
	arduino-cli compile --fqbn esp32:esp32:esp32 .

