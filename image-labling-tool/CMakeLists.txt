cmake_minimum_required(VERSION 3.16)

project(ImageLabellingTool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent)

add_subdirectory(autolabeling)

set(PROJECT_SOURCES
    main.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent)
    qt_add_resources(RESOURCES resources.qrc)

    qt_add_executable(ImageLabellingTool
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

        common/log/Logger.cpp
        common/log/Logger.h

        util/util.cpp
        util/util.h

        core/category/CategoryManager.cpp
        core/category/CategoryAction.h
        core/category/CategoryManager.h
        core/category/Category.cpp
        core/category/CategoryAction.cpp
        core/category/Category.h
        core/label/Label.h
        core/label/Label.cpp
        core/label/LabelAction.h
        core/label/LabelAction.cpp
        core/label/LabelManager.cpp
        core/label/LabelManager.h
        core/action/Action.cpp
        core/action/Action.h
        core/property/Property.cpp
        core/property/Property.h
        core/property/PropertyUpdateHandler.cpp
        core/property/PropertyUpdateHandler.h
        core/task/TaskManager.cpp
        core/task/AnnotationTypes.h
        core/task/TaskManager.h
        core/file/DatasetFile.h
        core/file/DatasetFile.cpp
        core/file/FileAction.cpp
        core/file/FileAction.h

        infra/DataLoader.h
        infra/DataSaver.h
        infra/DataManager.cpp
        infra/DataOperator.h
        infra/DataOperator.cpp
        infra/DataManager.h

        ui/common/graphics/TextWithBackgroundGraphicsItem.cpp
        ui/common/graphics/TextWithBackgroundGraphicsItem.h
        ui/common/graphics/VertexHandle.h
        ui/common/graphics/VertexHandle.cpp
        ui/common/widgets/LabelCategoryItemWidget.h
        ui/common/widgets/LabelCategoryItemWidget.cpp
        ui/label/LabelGraphicsItem.cpp
        ui/label/LabelItemFactory.cpp
        ui/label/LabelItemFactory.h
        ui/label/LabelItemWidget.cpp
        ui/label/LabelGraphicsItem.h
        ui/label/LabelItemWidget.h
        ui/label/UiProperty.h
        ui/dialog/category_selection/CategorySelectionDialog.h
        ui/dialog/category_selection/CategorySelectionDialog.cpp
        ui/dialog/category_reorder/CategoryReorderList.cpp
        ui/dialog/category_reorder/CategoryReorderList.h
        ui/dialog/category_reorder/CategoryReorderItem.h
        ui/dialog/category_reorder/CategoryReorderDialog.h
        ui/dialog/category_reorder/CategoryReorderDialog.cpp
        ui/dialog/category_reorder/CategoryReorderItem.cpp
        ui/dialog/task_selection/TasksDialog.cpp
        ui/dialog/task_selection/TaskCard.cpp
        ui/dialog/task_selection/TasksDialog.h
        ui/dialog/task_selection/TaskCard.h
        ui/dialog/autolabeling/AutoLabelingDialog.h
        ui/dialog/autolabeling/AutoLabelingDialog.cpp
        ui/dialog/augment/augmentdialog.h
        ui/dialog/augment/augmentdialog.cpp
        ui/dialog/augment/imagetiler.h
        ui/dialog/augment/imagetiler.cpp
        ui/forms/forms.h
        ui/enum/InteractionMode.h
        ui/enum/DrawState.h
        ui/components/label_list/LabelListInteractionFilter.h
        ui/components/label_list/LabelListFrame.cpp
        ui/components/label_list/LabelListFrame.h
        ui/components/label_list/LabelListInteractionFilter.cpp
        ui/components/file_list/FileListFrame.cpp
        ui/components/file_list/FileListFrame.h
        ui/components/annotation_widget/AnnotationWidgetBase.cpp
        ui/components/annotation_widget/AnnotationWidgetBase.h
        ui/components/label_editor/LabelSceneInteractionHandler.cpp
        ui/components/label_editor/ViewInteractionHandler.h
        ui/components/label_editor/LabelEditorFrame.cpp
        ui/components/label_editor/ViewInteractionHandler.cpp
        ui/components/label_editor/LabelScene.h
        ui/components/label_editor/LabelSceneInteractionHandler.h
        ui/components/label_editor/LabelEditorFrame.h
        ui/components/label_editor/LabelScene.cpp
        ui/components/control/ControlFrame.h
        ui/components/control/ControlFrame.cpp
        ui/components/category_list/AddNewCategoryDialog.cpp
        ui/components/category_list/CategoryItemWidget.cpp
        ui/components/category_list/CategoryListFrame.cpp
        ui/components/category_list/AddNewCategoryDialog.h
        ui/components/category_list/CategoryListFrame.h
        ui/components/category_list/CategoryItemWidget.h

        modality/image_annotation/common/ImageViewInteractionHandler.cpp
        modality/image_annotation/common/ImageAnnotationEditorFrame.cpp
        modality/image_annotation/common/ImageViewInteractionHandler.h
        modality/image_annotation/common/ImageAnnotationEditorFrame.h
        modality/image_annotation/detection/ui/label/DetectionLabelGraphicsItem.cpp
        modality/image_annotation/detection/ui/label/DetectionLabelItemFactory.h
        modality/image_annotation/detection/ui/label/DetectionLabelItemFactory.cpp
        modality/image_annotation/detection/ui/label/DetectionLabelGraphicsItem.h
        modality/image_annotation/detection/ui/dialog/DetectionShortcutsDialog.h
        modality/image_annotation/detection/ui/dialog/DetectionShortcutsDialog.cpp
        modality/image_annotation/detection/ui/components/annotation_widget/DetectionAnnotationWidget.h
        modality/image_annotation/detection/ui/components/annotation_widget/DetectionAnnotationWidget.cpp
        modality/image_annotation/detection/ui/components/label_editor/DetectionLabelEditHandler.h
        modality/image_annotation/detection/ui/components/label_editor/DetectionLabelDrawHandler.h
        modality/image_annotation/detection/ui/components/label_editor/DetectionLabelEditorFrame.cpp
        modality/image_annotation/detection/ui/components/label_editor/DetectionLabelScene.cpp
        modality/image_annotation/detection/ui/components/label_editor/DetectionLabelEditHandler.cpp
        modality/image_annotation/detection/ui/components/label_editor/DetectionLabelDrawHandler.cpp
        modality/image_annotation/detection/ui/components/label_editor/DetectionLabelEditorFrame.h
        modality/image_annotation/detection/ui/components/label_editor/DetectionLabelScene.h
        modality/image_annotation/detection/ui/components/label_list/DetectionLabelListFrame.h
        modality/image_annotation/detection/ui/components/label_list/DetectionLabelListFrame.cpp
        modality/image_annotation/detection/ui/components/control/DetectionControlFrame.cpp
        modality/image_annotation/detection/ui/components/control/DetectionControlFrame.h
        modality/image_annotation/detection/core/DetectionLabel.h
        modality/image_annotation/detection/core/DetectionLabel.cpp
        modality/image_annotation/detection/infra/DetectionDependenciesFactory.cpp
        modality/image_annotation/detection/infra/DetectionDependenciesFactory.h
        modality/image_annotation/detection/model/ans_yolo/AnsYoloDetection.h
        modality/image_annotation/detection/model/ans_yolo/infra/defines.h
        modality/image_annotation/detection/model/ans_yolo/infra/AnsYoloDetectionDataOperator.h
        modality/image_annotation/detection/model/ans_yolo/infra/AnsYoloDetectionDataSaver.cpp
        modality/image_annotation/detection/model/ans_yolo/infra/AnsYoloDetectionDataOperator.cpp
        modality/image_annotation/detection/model/ans_yolo/infra/AnsYoloDetectionDataLoader.h
        modality/image_annotation/detection/model/ans_yolo/infra/AnsYoloDetectionDataSaver.h
        modality/image_annotation/detection/model/ans_yolo/infra/AnsYoloDetectionDataLoader.cpp
        modality/image_annotation/detection/util/util.h
        modality/image_annotation/detection/util/util.cpp

        modality/image_annotation/core/ImageDatasetFile.cpp
        modality/image_annotation/core/ImageDatasetFile.h
        modality/image_annotation/image_processing/Gradient.cpp
        modality/image_annotation/image_processing/Lab.h
        modality/image_annotation/image_processing/Lab.cpp
        modality/image_annotation/image_processing/Gradient.h
        modality/image_annotation/image_processing/fill/Filler.h
        modality/image_annotation/image_processing/fill/ScanlineFiller.cpp
        modality/image_annotation/image_processing/fill/ScanlineFiller.h
        modality/image_annotation/image_processing/fill/Filler.cpp

        cmd/App.h
        cmd/MainWindow.h
        cmd/AnnotationWidgetFactory.cpp
        cmd/MainWindow.cpp
        cmd/AnnotationWidgetFactory.h
        cmd/App.cpp
    )

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ImageLabellingTool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ImageLabellingTool SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ImageLabellingTool
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(ImageLabellingTool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Concurrent)
target_link_libraries(ImageLabellingTool PRIVATE Qt6::Core Qt6::Concurrent)
target_link_libraries(ImageLabellingTool PRIVATE autolabel ${OpenCV_LIBS})
target_sources(ImageLabellingTool PRIVATE ${RESOURCES})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ImageLabellingTool)
endif()
set_target_properties(ImageLabellingTool PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ImageLabellingTool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ImageLabellingTool)
endif()
