#!/usr/bin/python
#

from __future__ import print_function
from operator import itemgetter, attrgetter
import sys
import re
import copy

infile = 'm68kdasm_in.lst';
outfile = 'm68kdasm_ss.txt';

class ReadFile:
    def __init__(self, inpath, outpath):
        try:
            fi = open(inpath, 'r')
        except Exception:
            err = sys.exc_info()[1]
            sys.stderr.write('cannot read file %s [%s]\n' % (inpath, err))
            sys.exit(1)

        self.lines = [[]for n in range(16)]
        for line in fi:
            line = line.rstrip()
            if len(line) < 2:
                pass
            elif line[0] == '\t' and line[1] == '{':
                line = re.sub('[\t\{\}]', '', line)   # cut tab, { and }
                items = re.split('\s*\,\s*', line)  # split space + comma + space
                if len(items[2]) > 2:
                    n = int(items[2][2], 16)
                    self.lines[n].append(items)

        # sort by match
        for n in range(16):
            self.lines[n].sort(key = itemgetter(2))
            self.lines[n].sort(key = itemgetter(1), reverse = True)

        try:
            fo = open(outpath, 'w')
        except Exception:
            err = sys.exc_info()[1]
            sys.stderr.write('cannot write file %s [%s]\n' % (outpath, err))
            sys.exit(1)

        for n in range(16):
            fo.writelines(['----',str(n),'\n'])
            for line in self.lines[n]:
                fo.writelines([line[2],', ',line[1],', ',line[0],'\n'])
#
#
#
def main(argv):
    rf = ReadFile(infile, outfile)


if __name__ == '__main__':
    sys.exit(main(sys.argv))
