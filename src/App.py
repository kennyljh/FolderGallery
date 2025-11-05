from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
import os
import sys
import subprocess

version = "0.1.0"
savedDir = ""

class App(QMainWindow):

    def __init__(self):
        super().__init__()

        self.setWindowTitle("FolderGallery " + version)
        self.resize(1480, 900)
        self.setStyleSheet("""
            background: white;
        """)

        toolbar = self.addToolBar("Toolbar")
        toolbar.setMovable(False)

        openAction = QAction("Open", self)
        openAction.triggered.connect(self.openFileDirectory)
        toolbar.addAction(openAction)

        self.scrollArea = QScrollArea(self)
        self.scrollArea.setWidgetResizable(True)
        scrollWidget = QWidget()
        self.layout = QGridLayout(scrollWidget)
        self.scrollArea.setWidget(scrollWidget)

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.scrollArea)

        centralWidget = QWidget(self)
        centralWidget.setStyleSheet("""
            background: #212121;
        """)
        centralWidget.setLayout(mainLayout)
        self.setCentralWidget(centralWidget)

        if savedDir:
            self.displayFirstImage(savedDir)

    def openFileDirectory(self):
        
        dir = QFileDialog.getExistingDirectory(self, "Select Directory", QDir.rootPath())

        if dir:
            self.saveFileDirectory(dir)

    def saveFileDirectory(self, dir):
        savedDir = dir
        self.setWindowTitle("FolderGallery " + version + " - " + savedDir)
        self.displayFirstImage(savedDir)
        # alert = QMessageBox()
        # alert.setText("Directory selected " + dir)
        # alert.exec()

    def displayFirstImage(self, parentDir):

        for i in reversed(range(self.layout.count())):
            widget = self.layout.itemAt(i).widget()

            if widget is not None:
                widget.deleteLater()

        subDirs = [os.path.join(parentDir, f) for f in os.listdir(parentDir)
                   if os.path.isdir(os.path.join(parentDir, f))]
        
        supportedExtensions = ['.jpg', '.jpeg', '.png', '.gif', '.webp']

        row = 0
        col = 0

        for subDir in subDirs:
            imgFiles = [f for f in os.listdir(subDir) if os.path.splitext(f)[1].lower() in supportedExtensions]

            if imgFiles:
                
                firstImgPath = os.path.join(subDir, imgFiles[0])
                pixmap = QPixmap(firstImgPath)

                pixmap = pixmap.scaled(350, 350, Qt.KeepAspectRatio)

                imgLabel = QLabel(self)
                imgLabel.setPixmap(pixmap)
                imgLabel.setAlignment(Qt.AlignCenter)
                imgLabel.setStyleSheet("""
                    padding: 5px;
                    border: 1px solid white;
                    border-radius: 5px;
                """)

                goButton = QPushButton("Open")
                goButton.clicked.connect(lambda checked, destination=subDir: self.openDirInExplorer(destination))
                goButton.setStyleSheet("""
                    background: white;
                    color: black;
                    border-radius: 5px;
                    padding: 5px;
                    margin: 0px 0px 20px 0px;
                """)

                imgBtnLayout = QVBoxLayout()
                imgBtnLayout.addWidget(imgLabel)
                imgBtnLayout.addWidget(goButton)

                self.layout.addLayout(imgBtnLayout, row, col)

                col += 1
                if col > 3:
                    col = 0
                    row += 1

    def openDirInExplorer(self, dir):
        # print(f"Going to this dir: {os.path.normpath(dir)}")
        subprocess.Popen(f'explorer "{os.path.normpath(dir)}"')

app = QApplication([])
window = App()
window.show()
app.exec_()