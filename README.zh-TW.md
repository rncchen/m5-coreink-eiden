# M5Stack CoreInk 鐵路發車看板模板

[English](README.md) | [日本語](README.ja.md)

這是一套供 M5Stack CoreInk 使用的離線鐵路發車看板模板。畫面可顯示雙向各兩班列車，支援自訂車種徽章、RTC 計時，以及透過 Wi-Fi 與 NTP 定期校時。

本儲存庫刻意不收錄真實營運業者、車站或時刻表資料。版本控制內只有虛構範例，使用者需自行填入依法可使用的資料。

## 功能

- 以資料設定站名、目的地、方向、車種徽章及平日/週六/假日時刻表。
- 在 200 x 200 電子紙畫面顯示雙向各兩班列車。
- 支援切換車站、旋轉畫面 180 度及手動校時。
- 班次變更之間進入深度睡眠，並每日執行一次畫面深度清理。
- 上傳 LittleFS 前可先驗證本機 JSON。
- Wi-Fi 憑證與使用者填寫的鐵路資料均由 Git 忽略。

## 需求

- M5Stack CoreInk。
- Python 3.10 以上。
- [PlatformIO Core](https://docs.platformio.org/en/latest/core/index.html)。
- 可供燒錄的 USB 連線。

安裝驗證工具所需套件：

```powershell
python -m pip install -r scripts/requirements.txt
```

## 快速開始

1. 將虛構範例複製成不受版本控制的工作檔：

   ```powershell
   Copy-Item data/stations.example.json data/stations.json
   Copy-Item data/tt/sample-central.example.json data/tt/sample-central.json
   Copy-Item data/tt/sample-north.example.json data/tt/sample-north.json
   ```

2. 將範例站點識別、顯示名稱、目的地、日期與班次換成自己的資料。每個時刻表檔名必須與站點識別相同。

3. 驗證資料：

   ```powershell
   python scripts/validate_data.py
   ```

4. 需要網路校時時，建立本機 Wi-Fi 設定：

   ```powershell
   Copy-Item data/wifi.example.json data/wifi.json
   ```

5. 建置並上傳檔案系統及韌體：

   ```powershell
   python -m platformio run -e coreink
   python -m platformio run -e coreink -t uploadfs
   python -m platformio run -e coreink -t upload
   ```

尚未建立本機資料時，可直接驗證版本控制內的範例：

```powershell
python scripts/validate_data.py --example
```

## 操作方式

| 操作 | 功能 |
| --- | --- |
| 搖桿上 | 上一站 |
| 搖桿下 | 下一站 |
| 按下搖桿 | 畫面旋轉 180 度 |
| 上方按鈕 | 透過 Wi-Fi 與 NTP 校時 |

畫面上半部對應 `down`，下半部對應 `up`。終點站可只設定其中一個方向，未使用的半部會顯示 `終着駅`。

## 資料格式

`data/stations.json` 存放共用中繼資料與站點定義；`data/tt/<station-id>.json` 存放每站六組班次，也就是 `up`/`down` 分別搭配 `wk`/`sa`/`ho`。

每筆班次使用精簡陣列：

```json
[815, 1, 0]
```

三個值依序為 `[HHMM, destination_index, vehicle_type_index]`，索引對應 `stations.json` 內陣列的順序。完整規格請參閱[資料撰寫指南](docs/data-authoring.md)。

## 資料授權與責任

可公開查閱的資訊，不代表可以複製、自動擷取、修改或再次散布。加入資料前，請確認營運業者的使用條款、資料庫權利、著作權規定，以及所在司法管轄區適用的 API 或開放資料授權。

來源要求標示出處時，請自行保存來源與標示資訊。資料授權未允許在儲存庫再次散布時，請勿提交該資料。本專案作者不提供鐵路資料，也不代為確認個別資料的法律授權。

## 文件

- [架構說明](docs/architecture.md)
- [資料撰寫指南](docs/data-authoring.md)
- [發布檢查清單](docs/release-checklist.md)

## 測試

```powershell
python -m pytest scripts/test_validate_data.py
python -m platformio test -e native
python -m platformio run -e coreink
```

## 授權

原始碼與虛構模板採用 [MIT License](LICENSE)，內含相依程式的授權請參閱[第三方聲明](THIRD_PARTY_NOTICES.md)。使用者建立的鐵路資料仍受各自來源條款與授權限制。
