# M5Stack CoreInk 鉄道発車案内テンプレート

[English](README.md) | [繁體中文](README.zh-TW.md)

M5Stack CoreInk 向けのオフライン鉄道発車案内テンプレートです。上下各 2 本の列車、設定可能な列車種別バッジ、RTC 時計、Wi-Fi と NTP による定期時刻同期に対応します。

このリポジトリには、実在する事業者、駅、時刻表のデータを収録していません。追跡対象の JSON は架空のサンプルです。利用者が、適法に利用できるデータを用意してください。

## 機能

- 駅名、行先、方向、列車種別バッジ、平日/土曜/休日ダイヤをデータで設定。
- 200 x 200 電子ペーパーに上下各 2 本の発車を表示。
- 駅切替、画面の 180 度回転、手動時刻同期。
- 発車時刻の間はディープスリープし、1 日 1 回画面を深くクリーニング。
- LittleFS へ転送する前にローカル JSON を検証。
- Wi-Fi 認証情報と利用者作成の鉄道データを Git の対象外に設定。

## 必要環境

- M5Stack CoreInk。
- Python 3.10 以降。
- [PlatformIO Core](https://docs.platformio.org/en/latest/core/index.html)。
- 書き込みに利用できる USB 接続。

検証用依存パッケージをインストールします。

```powershell
python -m pip install -r scripts/requirements.txt
```

## クイックスタート

1. 架空のサンプルを Git 対象外の作業ファイルへコピーします。

   ```powershell
   Copy-Item data/stations.example.json data/stations.json
   Copy-Item data/tt/sample-central.example.json data/tt/sample-central.json
   Copy-Item data/tt/sample-north.example.json data/tt/sample-north.json
   ```

2. サンプルの駅 ID、表示名、行先、日付、発車時刻を自分のデータに置き換えます。各時刻表のファイル名は駅 ID と一致させてください。

3. データを検証します。

   ```powershell
   python scripts/validate_data.py
   ```

4. ネットワーク時刻同期を使う場合は、ローカルの Wi-Fi 設定を作成します。

   ```powershell
   Copy-Item data/wifi.example.json data/wifi.json
   ```

5. ファイルシステムとファームウェアをビルドして転送します。

   ```powershell
   python -m platformio run -e coreink
   python -m platformio run -e coreink -t uploadfs
   python -m platformio run -e coreink -t upload
   ```

ローカルデータを作る前に、追跡対象のサンプルだけを検証できます。

```powershell
python scripts/validate_data.py --example
```

## 操作

| 操作 | 動作 |
| --- | --- |
| スティック上 | 前の駅 |
| スティック下 | 次の駅 |
| スティック押下 | 画面を 180 度回転 |
| 上ボタン | Wi-Fi と NTP で時刻同期 |

画面上半分は `down`、下半分は `up` に対応します。終着駅は片方向だけを定義でき、未使用側には `終着駅` が表示されます。

## データ形式

`data/stations.json` に共通メタデータと駅定義を保存します。`data/tt/<station-id>.json` には、`up`/`down` と `wk`/`sa`/`ho` を組み合わせた 6 種類の時刻表を保存します。

各発車は短い配列です。

```json
[815, 1, 0]
```

値は `[HHMM, destination_index, vehicle_type_index]` の順です。インデックスは `stations.json` 内の配列順を参照します。完全な仕様は[データ作成ガイド](docs/data-authoring.md)を参照してください。

## データの権利と責任

公開閲覧できる情報であっても、複製、自動取得、改変、再配布が許諾されているとは限りません。データを追加する前に、事業者の利用規約、データベース権、著作権、API またはオープンデータライセンスを確認してください。

出典表示が必要な場合は、利用者が出典記録を保存してください。リポジトリでの再配布が許可されていないデータはコミットしないでください。本プロジェクトの作者は鉄道データも個別の法的許諾も提供しません。

## ドキュメント

- [アーキテクチャ](docs/architecture.md)
- [データ作成ガイド](docs/data-authoring.md)
- [リリースチェックリスト](docs/release-checklist.md)

## テスト

```powershell
python -m pytest scripts/test_validate_data.py
python -m platformio test -e native
python -m platformio run -e coreink
```

## ライセンス

ソースコードと架空のテンプレートは [MIT License](LICENSE) で提供されます。同梱依存関係のライセンスは[サードパーティー通知](THIRD_PARTY_NOTICES.md)を参照してください。利用者が作成した鉄道データには、それぞれの出典条件とライセンスが引き続き適用されます。
