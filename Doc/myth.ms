
\# Create a PDF from this file:
\#  groff -Tpdf -tbl -ms -U -s  myth.ms > myth.pdf
\#  -g option runs gsoelim to pre-process include files
\#  pdfinfo seems to be required for embedding PDFs

.so front.ms
.so intro.ms
.so opcodes.ms
.so comms.ms
.so control.ms
.so assem.ms
