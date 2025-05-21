import os
import cv2
import csv
from glob import glob

# 読み込むディレクトリ（例: ./images フォルダ）
image_dir = '../trimming_image'

# 検出する画像ファイル（*.jpg）
image_files = glob(os.path.join(image_dir, '*.jpg'))

# 出力CSVファイル名
output_csv = '../trimming_image/image_sizes.csv'

# CSVに書き込む（ファイル名, 幅, 高さ）
with open(output_csv, 'w', newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(['filename', 'width', 'height'])  # ヘッダ行

    for image_path in image_files:
        img = cv2.imread(image_path)
        if img is None:
            print(f"読み込み失敗: {image_path}")
            continue
        height, width = img.shape[:2]
        filename = os.path.basename(image_path)
        writer.writerow([filename, width, height])

print(f"CSVファイルに保存しました: {output_csv}")
