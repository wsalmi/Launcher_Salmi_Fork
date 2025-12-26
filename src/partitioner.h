#ifndef __PARTITIONER_H
#define __PARTITIONER_H

#include <FS.h>
#include <esp_flash.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

void partitioner();

void partList();

void dumpPartition(const char *partitionLabel, const char *outputPath);

void restorePartition(const char *partitionLabel);

bool attachPartition(String from, String to);

void partitionCrawler();
bool partitionSetter(const uint8_t *scheme, size_t scheme_size);
#endif /*__PARTITIONER_H*/
