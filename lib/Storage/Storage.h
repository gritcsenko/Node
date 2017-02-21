#include <SD.h>

bool InitSD(bool testCard = true)
{
  Serial.println("Initializing SD card...");
  if(testCard){
    Sd2Card card;
    SdVolume volume;
    SdFile root;

    // we'll use the initialization code from the utility libraries
    // since we're just testing if the card is working!
    if (!card.init(SPI_HALF_SPEED, D8)) {
      Serial.println("initialization failed. Things to check:");
      Serial.println("* is a card inserted?");
      Serial.println("* is your wiring correct?");
      Serial.println("* did you change the chipSelect pin to match your shield or module?");
      return false;
    } else {
      Serial.println("Wiring is correct and a card is present.");
    }

    // print the type of card
    Serial.print("\nCard type: ");
    switch (card.type()) {
      case SD_CARD_TYPE_SD1:
        Serial.println("SD1");
        break;
      case SD_CARD_TYPE_SD2:
        Serial.println("SD2");
        break;
      case SD_CARD_TYPE_SDHC:
        Serial.println("SDHC");
        break;
      default:
        Serial.println("Unknown");
    }


    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    if (!volume.init(card)) {
      Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
      return false;
    }


    // print the type and size of the first FAT-type volume
    uint32_t volumesize;
    Serial.print("\nVolume type is FAT");
    Serial.println(volume.fatType(), DEC);

    uint32_t bpc = volume.blocksPerCluster();
    uint32_t clusters = volume.clusterCount();

    Serial.print("Clusters: ");
    Serial.println(clusters, DEC);
    Serial.print("Blocks: ");
    Serial.println(bpc * clusters, DEC);
    Serial.print("Blocks per cluster: ");
    Serial.println(bpc, DEC);
    Serial.println("Block size: 512");
    Serial.print("Cluster size: ");
    Serial.println(bpc * 512, DEC);
    Serial.println();


    volumesize = bpc;        // clusters are collections of blocks
    volumesize *= clusters;  // we'll have a lot of clusters
    volumesize *= 512;       // SD card blocks are always 512 bytes
    Serial.print("Volume size (bytes): ");
    Serial.println(volumesize);
    Serial.print("Volume size (Kbytes): ");
    float kb = volumesize / 1024.0;
    Serial.println(kb);
    Serial.print("Volume size (Mbytes): ");
    float mb = volumesize / (1024.0 * 1024);
    Serial.println(mb);
    Serial.print("Volume size (Gbytes): ");
    float gb = volumesize / (1024.0 * 1024 * 1024);
    Serial.println(gb);

    Serial.println("\nFiles found on the card (name, date and size in bytes): ");
    root.openRoot(volume);

    // list all files in the card with date and size
    root.ls(LS_R | LS_DATE | LS_SIZE, 2);

    root.close();
  }

  SD.begin(D8);
  return true;
}
