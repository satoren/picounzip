import zipfile
import os
import tempfile

import random

def zip_archive_with_dir(zipfilename,arcdir):
    with zipfile.ZipFile(zipfilename, 'w',zipfile.ZIP_DEFLATED) as myzip:
        for root, dirs, files in os.walk(arcdir):
            for file in files:
                filename = os.path.join(root, file)
                myzip.write(filename,os.path.relpath(filename, arcdir))


with tempfile.TemporaryDirectory() as largedata_tempdir:
    with open(os.path.join(largedata_tempdir,'zero_largedata'),'wb') as f:
        for i in range(1024 * 6):
            f.write(bytes(1024 * 1024))
    zip_archive_with_dir('resource/large_data/zero_largedata.zip',largedata_tempdir)


with tempfile.TemporaryDirectory() as large_entry_tempdir:
    for i in range(100000):
        with open(os.path.join(large_entry_tempdir,'file' + str(i)),'wb') as f:
            pass
    zip_archive_with_dir('resource/large_data/large_entry.zip',large_entry_tempdir)


     