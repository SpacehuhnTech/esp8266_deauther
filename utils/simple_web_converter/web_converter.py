import gzip
from os import path
import argparse
import css_minifier
import html_minifier

def CompressedGZipArray(data:bytes) -> list:
    comp=gzip.compress(data)
    return [hex(i) for i in comp]

def CompressedGZipArrayFromFile(fp) -> list:
    data=fp.read()
    if type(data)==type(b''):
        return CompressedGZipArray(data)
    else:
        return CompressedGZipArray(bytes(data))

def ExportCompressedFile(infile:str,outpath:str='./gz/'):
    with open(infile,'r') as plain:
        if infile.endswith(".html"):
            minified=html_minifier.html_minify(plain.read())
        elif infile.endswith(".css"):
            minified=css_minifier.css_minify(plain.read())
        else:
            minified=plain.read()
        
        with gzip.open(path.join(path.dirname(infile),outpath,path.basename(infile))+'.gz','wb+') as compressed:
            compressed.write(bytes(minified,encoding='utf-8'))
            print("const char "+path.basename(infile).replace('.','').lower()+'[] PROGMEM = {'+' ,'.join(CompressedGZipArray(bytes(minified,encoding='utf-8')))+'};')
    


if __name__=="__main__":
    parser=argparse.ArgumentParser("ESP8266 Deauther Simple Web Converter",description="Converts the html or css to ESP8266 Deauther Friendly GZip files and exports the GZip Array",usage="web_converter.py [-h] [-O OUT] FILE")
    parser.add_argument("FILE",help="Path of the HTML or CSS file to convert")
    parser.add_argument("-O","--out",help="Output path where the converted file will be stored")
    arguments=parser.parse_args()
    inputFile=arguments.FILE
    outputPath=arguments.out if arguments.out else './gz/'
    ExportCompressedFile(inputFile,outputPath)
