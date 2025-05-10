# 

<div align="center">
   <h1>C++_IDcardLiveness_AI_WebServer</h1>
</div>



### 1. Core Architecture

This code implements a **ID Card Liveness Detection Http Server** with three main components:

1. **License Management System** (10s interval check)
2. **Document SDK Integration** (8 attack-specific detection pipelines)
3. **REST API Service** (JSON input/output)

------

### 2. Key Technical Features

#### 2.1 Pipeline Configuration

| Pipeline Name       | Attack Type              | Array Index |
| ------------------- | ------------------------ | ----------- |
| toucan-sr           | Screen Replay Detection  | 0           |
| robin-pc            | Printed Copy Detection   | 3           |
| umbrellabird-ps     | Portrait Substitution    | 5           |
| [7 other pipelines] | Various document attacks | 1-7         |

#### 2.2 Detection Workflow

```
1. License Validation → 
2. Image Acquisition (Multipart/Base64) → 
3. Temporary File Storage → 
4. Pipeline Selection → 
5. SDK Processing → 
6. JSON Result Aggregation
```

#### 2.3 Error Code Handling

```
kDocSdkValidationStatusCodeDocumentTooCloseToBorder → "Document too close to border"
kDocSdkValidationStatusCodeDocumentIsColorless → "Document is colorless"
// 8+ specific error conditions handled
```

------

### 3. Core Components

#### 3.1 License Management

- 10-second interval check via `TF_READ_LIC` thread
- Cross-platform time handling (Windows/Linux)
- Critical section protection for license data

#### 3.2 SDK Integration

```
typedef struct {
    float liveness_probability;
    float quality_score;
    DocSdkValidationStatusCode status_code;
} DocSdkLivenessPipelineResult;
```

#### 3.3 Pipeline Initialization

```
void ClaHTTPServerWrapper::launch() {
    memset(g_pipeline_List, 0, ...);
    GetSubProc("toucan-sr", "", 1); // Pre-init pipelines
    GetSubProc("robin-pc", "", 1);
    GetSubProc("umbrellabird-ps", "", 1);
}
```

------

### 4. Result

<div align="center">
   <img src=https://github.com/LucaIT523/C_IDcardLiveness_AI_WebServer/blob/main/images/1.png>
</div>




### **Contact Us**

For any inquiries or questions, please contact us.

telegram : @topdev1012

email :  skymorning523@gmail.com

Teams :  https://teams.live.com/l/invite/FEA2FDDFSy11sfuegI