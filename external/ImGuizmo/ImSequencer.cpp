// https://github.com/CedricGuillemet/ImGuizmo
// v 1.84 WIP
//
// The MIT License(MIT)
//
// Copyright(c) 2021 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "ImSequencer.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <cmath>
#include <cstdlib>
#include <float.h>

namespace ImSequencer
{
#ifndef IMGUI_DEFINE_MATH_OPERATORS
   static ImVec2 operator+(const ImVec2& a, const ImVec2& b) {
      return ImVec2(a.x + b.x, a.y + b.y);
   }
#endif
   static bool SequencerAddDelButton(ImDrawList* draw_list, ImVec2 pos, bool add = true)
   {
      ImGuiIO& io = ImGui::GetIO();
      ImRect delRect(pos, ImVec2(pos.x + 16, pos.y + 16));
      bool overDel = delRect.Contains(io.MousePos);
      int delColor = overDel ? 0xFFAAAAAA : 0x77A3B2AA;
      float midy = pos.y + 16 / 2 - 0.5f;
      float midx = pos.x + 16 / 2 - 0.5f;
      draw_list->AddRect(delRect.Min, delRect.Max, delColor, 4);
      draw_list->AddLine(ImVec2(delRect.Min.x + 3, midy), ImVec2(delRect.Max.x - 3, midy), delColor, 2);
      if (add)
         draw_list->AddLine(ImVec2(midx, delRect.Min.y + 3), ImVec2(midx, delRect.Max.y - 3), delColor, 2);
      return overDel;
   }

   bool Sequencer(SequenceInterface* sequence, double zoom, int currentBeatsplit, int beatsPerMeasure, bool darkTheme, bool haveSelection, bool windowFocused, bool* expanded,
                  bool* updatedBeat, bool* leftClickedEntity, bool* leftClickReleased, bool* leftClickShift, bool* rightClickedEntity, double* clickedBeat,
                  double* hoveredBeat, int* clickedItemType, int* releasedItemType, int* selectedEntry, double* firstFrame, int sequenceOptions)
   {
      bool ret = false;
      ImGuiIO& io = ImGui::GetIO();
      int cx = (int)(io.MousePos.x);
      int cy = (int)(io.MousePos.y);
      static float framePixelWidth = 64.f;
      static float framePixelWidthTarget = 64.f;
      int legendWidth = 200;

      framePixelWidthTarget = 64.f * zoom;

      float beatsplitPixelWidth = framePixelWidth / currentBeatsplit;

      static float firstClickedBeat = 0;

      static int movingEntry = -1;
      static int movingPos = -1;
      static int movingPart = -1;
      int delEntry = -1;
      int dupEntry = -1;
      int ItemHeight = 20;

      bool popupOpened = false;
      int sequenceCount = sequence->GetItemTypeCount();
      if (!sequenceCount)
         return false;
      ImGui::BeginGroup();

      ImDrawList* draw_list = ImGui::GetWindowDrawList();
      ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
      ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
      float firstFrameUsed = firstFrame ? *firstFrame : 0;


      int controlHeight = sequenceCount * ItemHeight;
      for (int i = 0; i < sequenceCount; i++)
         controlHeight += int(sequence->GetCustomHeight(i));
      int frameCount = ImMax(sequence->GetFrameMax() - sequence->GetFrameMin(), 1);

      if(updatedBeat)
         *updatedBeat = false;

      static bool MovingScrollBar = false;
      struct CustomDraw
      {
         int index;
         ImRect customRect;
         ImRect legendRect;
         ImRect clippingRect;
         ImRect legendClippingRect;
      };
      ImVector<CustomDraw> customDraws;
      ImVector<CustomDraw> compactCustomDraws;
      // zoom in/out
      const float visibleFrameCount = (canvas_size.x - legendWidth) / framePixelWidth;
      const float barWidthRatio = ImMin(visibleFrameCount / (float)frameCount, 1.f);
      const float barWidthInPixels = barWidthRatio * (canvas_size.x - legendWidth);

      const float visibleBeatsplitCount = visibleFrameCount * currentBeatsplit;

      ImRect regionRect(canvas_pos, canvas_pos + canvas_size);

      static bool panningView = false;
      static ImVec2 panningViewSource;
      static float panningViewFrame;
      if (ImGui::IsWindowFocused() && io.KeyAlt && io.MouseDown[2])
      {
         if (!panningView)
         {
            panningViewSource = io.MousePos;
            panningView = true;
            panningViewFrame = *firstFrame;
         }
         *firstFrame = panningViewFrame - ((io.MousePos.x - panningViewSource.x) / framePixelWidth);
         *firstFrame = ImClamp(*firstFrame, (double)sequence->GetFrameMin(), (double)sequence->GetFrameMax() - visibleFrameCount);
         
         if(updatedBeat)
            *updatedBeat = true;
      }
      if (panningView && !io.MouseDown[2])
      {
         panningView = false;
      }
      framePixelWidthTarget = ImClamp(framePixelWidthTarget, 0.25f, 512.f);

      framePixelWidth = ImLerp(framePixelWidth, framePixelWidthTarget, 0.33f);

      frameCount = sequence->GetFrameMax() - sequence->GetFrameMin();
      if (visibleFrameCount >= (float)frameCount && firstFrame) {
         *firstFrame = sequence->GetFrameMin();
         if(updatedBeat)
            *updatedBeat = true;
      }


      // --
      if (expanded && !*expanded)
      {
         ImGui::InvisibleButton("canvas", ImVec2(canvas_size.x - canvas_pos.x, (float)ItemHeight));
         draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);
         char tmps[512];
         ImFormatString(tmps, IM_ARRAYSIZE(tmps), sequence->GetCollapseFmt(), frameCount, sequenceCount);
         draw_list->AddText(ImVec2(canvas_pos.x + 26, canvas_pos.y + 2), 0xFFFFFFFF, tmps);
      }
      else
      {
         bool hasScrollBar(true);
         /*
         int framesPixelWidth = int(frameCount * framePixelWidth);
         if ((framesPixelWidth + legendWidth) >= canvas_size.x)
         {
             hasScrollBar = true;
         }
         */
         // test scroll area
         ImVec2 headerSize(canvas_size.x, (float)ItemHeight);
         ImVec2 scrollBarSize(canvas_size.x, 14.f);
         ImGui::InvisibleButton("topBar", headerSize);
         draw_list->AddRectFilled(canvas_pos, canvas_pos + headerSize, 0xFFFF0000, 0);
         ImVec2 childFramePos = ImGui::GetCursorScreenPos();
         ImVec2 childFrameSize(canvas_size.x, canvas_size.y - 8.f - headerSize.y - (hasScrollBar ? scrollBarSize.y : 0));
         ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
         ImGui::BeginChildFrame(889, childFrameSize);
         sequence->focused = ImGui::IsWindowFocused();
         ImGui::InvisibleButton("contentBar", ImVec2(canvas_size.x, float(controlHeight)));
         const ImVec2 contentMin = ImGui::GetItemRectMin();
         const ImVec2 contentMax = ImGui::GetItemRectMax();
         const ImRect contentRect(contentMin, contentMax);
         const float contentHeight = contentMax.y - contentMin.y;

         // full background
         unsigned int bgCol = darkTheme ? 0xFF242424 : 0xF5F5F5;
         draw_list->AddRectFilled(canvas_pos, canvas_pos + canvas_size, bgCol, 0);

         // current frame top
         ImRect topRect(ImVec2(canvas_pos.x + legendWidth, canvas_pos.y), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + ItemHeight));

         //header
         unsigned int headerCol = darkTheme ? 0xFF3D3837 : 0xFFC5C5C5;
         draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), headerCol, 0);
         if (sequenceOptions & SEQUENCER_ADD)
         {
            if (SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + legendWidth - ItemHeight, canvas_pos.y + 2), true) && io.MouseReleased[0])
               ImGui::OpenPopup("addEntry");

            if (ImGui::BeginPopup("addEntry"))
            {
               for (int i = 0; i < sequence->GetItemTypeCount(); i++)
                  if (ImGui::Selectable(sequence->GetItemTypeName(i)))
                  {
                     sequence->Add(i);
                     *selectedEntry = sequence->GetItemCount() - 1;
                  }

               ImGui::EndPopup();
               popupOpened = true;
            }
         }

         //header frame number and lines
         int modFrameCount = beatsPerMeasure;
         int frameStep = 1;
         while ((modFrameCount * framePixelWidth) < 150)
         {
            modFrameCount *= 2;
            frameStep *= 2;
         };
         int halfModFrameCount = currentBeatsplit;

         auto drawLine = [&](int i, int regionHeight) {
            bool baseIndex = ((i % modFrameCount) == 0) || (i == sequence->GetFrameMax() || i == sequence->GetFrameMin());
            bool halfIndex = (i % halfModFrameCount) == 0;
            float px = canvas_pos.x + (i * framePixelWidth) + legendWidth - (firstFrameUsed * framePixelWidth);
            float tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
            float tiretEnd = baseIndex ? regionHeight : ItemHeight;

            if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
            {
               draw_list->AddLine(ImVec2(px, canvas_pos.y + (float)tiretStart), ImVec2(px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

               draw_list->AddLine(ImVec2(px, canvas_pos.y + (float)ItemHeight), ImVec2(px, canvas_pos.y + (float)regionHeight - 1), 0x30606060, 1);
            }

            if (baseIndex && px > (canvas_pos.x + legendWidth))
            {
               char tmps[512];
               ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", i);
               unsigned int textCol = darkTheme ? 0xFFBBBBBB : 0xFF111111;
               draw_list->AddText(ImVec2((float)px + 3.f, canvas_pos.y), textCol, tmps);
            }

         };

         auto drawLineContent = [&](int i, int /*regionHeight*/) {
            float px = canvas_pos.x + (i * framePixelWidth) + legendWidth - (firstFrameUsed * framePixelWidth);
            float tiretStart = contentMin.y;
            float tiretEnd = contentMax.y;

            if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
            {
               //draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

               draw_list->AddLine(ImVec2(px, tiretStart), ImVec2(px, tiretEnd), 0x30606060, 1);
            }
         };
         for (int i = sequence->GetFrameMin(); i <= sequence->GetFrameMax(); i += frameStep)
         {
            drawLine(i, ItemHeight);
         }
         drawLine(sequence->GetFrameMin(), ItemHeight);
         drawLine(sequence->GetFrameMax(), ItemHeight);
         /*
                  draw_list->AddLine(canvas_pos, ImVec2(canvas_pos.x, canvas_pos.y + controlHeight), 0xFF000000, 1);
                  draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + ItemHeight), ImVec2(canvas_size.x, canvas_pos.y + ItemHeight), 0xFF000000, 1);
                  */
                  // clip content

         draw_list->PushClipRect(childFramePos, childFramePos + childFrameSize);

         // draw item names in the legend rect on the left
         size_t customHeight = 0;
         for (int i = 0; i < sequenceCount; i++)
         {
            ImVec2 tpos(contentMin.x + 3, contentMin.y + i * ItemHeight + 2 + customHeight);
            unsigned int textCol = darkTheme ? 0xFFFFFFFF : 0xFF000000;
            draw_list->AddText(tpos, textCol, sequence->GetItemTypeName(i));

            if (sequenceOptions & SEQUENCER_DEL)
            {
               bool overDel = SequencerAddDelButton(draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight + 2 - 10, tpos.y + 2), false);
               if (overDel && io.MouseReleased[0])
                  delEntry = i;

               bool overDup = SequencerAddDelButton(draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight - ItemHeight + 2 - 10, tpos.y + 2), true);
               if (overDup && io.MouseReleased[0])
                  dupEntry = i;
            }
            customHeight += sequence->GetCustomHeight(i);
         }

         // clipping rect so items bars are not visible in the legend on the left when scrolled
         //

         // determine first selectable beat start
         float firstSelectableBeatX = contentMin.x + legendWidth;
         float firstSelectableBeat = firstFrame ? *firstFrame : 0;

         if(firstFrame && std::abs(*firstFrame / currentBeatsplit) > FLT_EPSILON) {
            int fullBeats = std::floor(*firstFrame);
            int fullBeatSplits = std::ceil((*firstFrame - fullBeats) / currentBeatsplit);

            firstSelectableBeat = fullBeats + (fullBeatSplits * (1.f / currentBeatsplit));
            firstSelectableBeatX += (firstSelectableBeat - *firstFrame) * framePixelWidth;
         }

         // slots background
         static float shiftClickY = 0;
         static float releasedBeat = 0;
         
         static float selectStartY = 0;
         static float selectEndY = 0;

         customHeight = 0;
         for (int i = 0; i < sequenceCount; i++)
         {
            unsigned int col = darkTheme ? ((i & 1) ? 0xFF3A3636 : 0xFF413D3D) : ((i & 1) ? 0xFFF5F5F5 : 0xFFBCB3B5);

            size_t localCustomHeight = sequence->GetCustomHeight(i);
            ImVec2 pos = ImVec2(contentMin.x + legendWidth, contentMin.y + ItemHeight * i + 1 + customHeight);
            ImVec2 sz = ImVec2(canvas_size.x + canvas_pos.x, pos.y + ItemHeight - 1 + localCustomHeight);
            
            draw_list->AddRectFilled(pos, sz, col, 0);
            customHeight += localCustomHeight;

            // highlight beatsplit mouse is hovered over
            if(windowFocused) {
               for(int j = 0; j < visibleBeatsplitCount; j++)
               {
                  if (!popupOpened && cy >= pos.y && cy < pos.y + (ItemHeight + localCustomHeight) && movingEntry == -1 &&
                     cx > firstSelectableBeatX + j * beatsplitPixelWidth && cx < firstSelectableBeatX + (j + 1) * beatsplitPixelWidth)
                  {
                     unsigned int hoverCol = darkTheme ? col + 0x80201008 : 0xFF767A93 + 0x80201008;
                     ImVec2 hoverPos = pos;
                     ImVec2 hoverSz = sz;

                     if(hoveredBeat) {
                        *hoveredBeat = firstSelectableBeat + (j * 1.f / currentBeatsplit);
                     }

                     // add event or modify existing event
                     if(io.MouseClicked[0] || io.MouseClicked[1] || io.MouseReleased[0]) {
                        if(clickedBeat) {
                           *clickedBeat = firstSelectableBeat + (j * 1.f / currentBeatsplit);
                        }

                        if(clickedItemType && !(leftClickShift && *leftClickShift)) {
                           *clickedItemType = i;
                        }

                        if(io.MouseClicked[0] && leftClickedEntity) {
                           *leftClickedEntity = true;

                           if(clickedBeat) {
                              firstClickedBeat = *clickedBeat;
                           }

                           if(leftClickReleased) {
                              *leftClickReleased = false;
                           }

                           if(leftClickShift && io.KeyShift) {
                              *leftClickShift = true;
                              shiftClickY = hoverPos.y;
                           }
                        }

                        if(leftClickReleased && ImGui::IsMouseReleased(0)) {
                           *leftClickReleased = true;

                           if(releasedItemType) {
                              *releasedItemType = i;
                           }
                        }

                        if(io.MouseClicked[1] && rightClickedEntity) {
                           *rightClickedEntity = true;
                        }
                     }

                     float draggedBeats = (firstSelectableBeat + (j * 1.f / currentBeatsplit)) - firstClickedBeat;

                     // light outline over currently hovered region
                     if((io.MouseDown[0] || (leftClickShift && *leftClickShift)) && draggedBeats > 0.f) {
                        hoverSz.x = firstSelectableBeatX + j * beatsplitPixelWidth - 3 + 32 - 3; 
                        hoverPos.x = hoverSz.x - (draggedBeats * currentBeatsplit) * beatsplitPixelWidth - 32 + 3; 
                     } else {
                        hoverPos.x = firstSelectableBeatX + j * beatsplitPixelWidth - 3;
                        hoverSz.x = hoverPos.x + 32 - 3;
                     }

                     if(leftClickShift && *leftClickShift && clickedItemType) {
                        if(i < *clickedItemType) {
                           hoverPos.y = hoverSz.y;
                           hoverSz.y = shiftClickY;
                        } else {
                           hoverPos.y = shiftClickY;
                        }
                     }

                     if(leftClickReleased && *leftClickReleased && leftClickShift && *leftClickShift) {
                        releasedBeat = *clickedBeat;
                        selectStartY = hoverPos.y - contentMin.y;
                        selectEndY = hoverSz.y - contentMin.y;
                     }

                     draw_list->AddRectFilled(hoverPos, hoverSz, hoverCol);

                     break;
                  }

               }
            }
         }

         draw_list->PushClipRect(childFramePos + ImVec2(float(legendWidth), 0.f), childFramePos + childFrameSize);

         // vertical frame lines in content area
         for (int i = sequence->GetFrameMin(); i <= sequence->GetFrameMax(); i += frameStep)
         {
            drawLineContent(i, int(contentHeight));
         }
         drawLineContent(sequence->GetFrameMin(), int(contentHeight));
         drawLineContent(sequence->GetFrameMax(), int(contentHeight));

         // selection
         bool selected = selectedEntry && (*selectedEntry >= 0);
         if (selected)
         {
            customHeight = 0;
            for (int i = 0; i < *selectedEntry; i++)
               customHeight += sequence->GetCustomHeight(i);;
            draw_list->AddRectFilled(ImVec2(contentMin.x, contentMin.y + ItemHeight * *selectedEntry + customHeight), ImVec2(contentMin.x + canvas_size.x, contentMin.y + ItemHeight * (*selectedEntry + 1) + customHeight), 0x801080FF, 1.f);
         }

         if(haveSelection && (!(*leftClickedEntity)) && windowFocused) {
            unsigned int selectCol = darkTheme ? 0xFF3A3636 + 0x80201008 : 0xFF767A93 + 0x80201008;

            ImVec2 startSel(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth + firstClickedBeat * framePixelWidth, contentMin.y + selectStartY);
            ImVec2 endSel(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth + releasedBeat * framePixelWidth + 32 - 3, contentMin.y + selectEndY);

            if (startSel.x <= (canvas_size.x + contentMin.x) && endSel.x >= (contentMin.x + legendWidth))
            {
               draw_list->AddRectFilled(startSel, endSel, selectCol, 2);
            }
         }

         // draw the sequence items
         for (int i = 0; i < sequence->GetItemCount(); i++)
         {
            double* start, * end;
            int itemType;
            unsigned int color = darkTheme ? 0xFFAA8080 : 0xFFBAD4F3;
            const char * displayText;
            sequence->Get(i, &start, &end, &itemType, nullptr, &displayText);
            size_t localCustomHeight = sequence->GetCustomHeight(i);

            customHeight = localCustomHeight * itemType;

            ImVec2 pos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + ItemHeight * itemType + 1 + customHeight);
            ImVec2 slotP1(pos.x + *start * framePixelWidth - 3, pos.y + 2);
            ImVec2 slotP2;
            
            if(start && end && std::abs(*start - *end) < DBL_EPSILON) {
               slotP2 = ImVec2(pos.x + *start * framePixelWidth + 32 - 3, pos.y + ItemHeight - 2 + localCustomHeight);
            } else {
               slotP2 = ImVec2(pos.x + *end * framePixelWidth + 32 - 3, pos.y + ItemHeight - 2 + localCustomHeight);
            }

            unsigned int slotColor = color | 0xFF000000;

            if (slotP1.x <= (canvas_size.x + contentMin.x) && slotP2.x >= (contentMin.x + legendWidth))
            {
               draw_list->AddRectFilled(slotP1, slotP2, slotColor, 2);
            }
            if (ImRect(slotP1, slotP2).Contains(io.MousePos) && io.MouseDoubleClicked[0])
            {
               sequence->DoubleClick(i);
            }
            // Ensure grabbable handles
            const float max_handle_width = slotP2.x - slotP1.x / 3.0f;
            const float min_handle_width = ImMin(10.0f, max_handle_width);
            const float handle_width = ImClamp(framePixelWidth / 2.0f, min_handle_width, max_handle_width);
            ImRect rects[3] = { ImRect(slotP1, ImVec2(slotP1.x + handle_width, slotP2.y))
                , ImRect(ImVec2(slotP2.x - handle_width, slotP1.y), slotP2)
                , ImRect(slotP1, slotP2) };

            const unsigned int quadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, slotColor + (selected ? 0 : 0x202020) };
            if (movingEntry == -1 && (sequenceOptions & SEQUENCER_EDIT_STARTEND))// TODOFOCUS && backgroundRect.Contains(io.MousePos))
            {
               for (int j = 2; j >= 0; j--)
               {
                  ImRect& rc = rects[j];
                  if (!rc.Contains(io.MousePos))
                     continue;
                  draw_list->AddRectFilled(rc.Min, rc.Max, quadColor[j], 2);
               }

               for (int j = 0; j < 3; j++)
               {
                  ImRect& rc = rects[j];
                  if (!rc.Contains(io.MousePos))
                     continue;
                  if (!ImRect(childFramePos, childFramePos + childFrameSize).Contains(io.MousePos))
                     continue;
                  if (ImGui::IsMouseClicked(0) && !MovingScrollBar)
                  {
                     movingEntry = i;
                     movingPos = cx;
                     movingPart = j + 1;
                     sequence->BeginEdit(movingEntry);
                     break;
                  }
               }
            }

            ImVec2 textPos = ImVec2(slotP1.x + 4, (slotP1.y + slotP2.y) / 2.f - (slotP2.y - slotP1.y) / 4.f);
            unsigned int noteTextCol = darkTheme ? 0xFFFFFFFF : 0xFF000000;
            draw_list->AddText(textPos, noteTextCol, displayText);

            // custom draw
            if (localCustomHeight > 0)
            {
               ImVec2 rp(canvas_pos.x, contentMin.y + ItemHeight * i + 1 + customHeight);
               ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - sequence->GetFrameMin() - 0.5f) * framePixelWidth, float(ItemHeight)),
                  rp + ImVec2(legendWidth + (sequence->GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(localCustomHeight + ItemHeight)));
               ImRect clippingRect(rp + ImVec2(float(legendWidth), float(ItemHeight)), rp + ImVec2(canvas_size.x, float(localCustomHeight + ItemHeight)));

               ImRect legendRect(rp + ImVec2(0.f, float(ItemHeight)), rp + ImVec2(float(legendWidth), float(localCustomHeight)));
               ImRect legendClippingRect(canvas_pos + ImVec2(0.f, float(ItemHeight)), canvas_pos + ImVec2(float(legendWidth), float(localCustomHeight + ItemHeight)));
               customDraws.push_back({ i, customRect, legendRect, clippingRect, legendClippingRect });
            }
            else
            {
               ImVec2 rp(canvas_pos.x, contentMin.y + ItemHeight * i + customHeight);
               ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - sequence->GetFrameMin() - 0.5f) * framePixelWidth, float(0.f)),
                  rp + ImVec2(legendWidth + (sequence->GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(ItemHeight)));
               ImRect clippingRect(rp + ImVec2(float(legendWidth), float(0.f)), rp + ImVec2(canvas_size.x, float(ItemHeight)));

               compactCustomDraws.push_back({ i, customRect, ImRect(), clippingRect, ImRect() });
            }
            //customHeight += localCustomHeight;
         }


         // moving
         if (/*backgroundRect.Contains(io.MousePos) && */movingEntry >= 0)
         {
            ImGui::CaptureMouseFromApp();
            int diffFrame = int((cx - movingPos) / framePixelWidth);
            if (std::abs(diffFrame) > 0)
            {
               double* start, * end;
               sequence->Get(movingEntry, &start, &end, NULL, NULL, NULL);
               if (selectedEntry)
                  *selectedEntry = movingEntry;
               double& l = *start;
               double& r = *end;
               if (movingPart & 1)
                  l += diffFrame;
               if (movingPart & 2)
                  r += diffFrame;
               if (l < 0)
               {
                  if (movingPart & 2)
                     r -= l;
                  l = 0;
               }
               if (movingPart & 1 && l > r)
                  l = r;
               if (movingPart & 2 && r < l)
                  r = l;
               movingPos += int(diffFrame * framePixelWidth);
            }
            if (!io.MouseDown[0])
            {
               // single select
               if (!diffFrame && movingPart && selectedEntry)
               {
                  *selectedEntry = movingEntry;
                  ret = true;
               }

               movingEntry = -1;
               sequence->EndEdit();
            }
         }

         draw_list->PopClipRect();
         draw_list->PopClipRect();

         for (const auto& customDraw : customDraws)
            sequence->CustomDraw(customDraw.index, draw_list, customDraw.customRect, customDraw.legendRect, customDraw.clippingRect, customDraw.legendClippingRect);
         for (const auto& customDraw : compactCustomDraws)
            sequence->CustomDrawCompact(customDraw.index, draw_list, customDraw.customRect, customDraw.clippingRect);

         // copy paste
         if (sequenceOptions & SEQUENCER_COPYPASTE)
         {
            ImRect rectCopy(ImVec2(contentMin.x + 100, canvas_pos.y + 2)
               , ImVec2(contentMin.x + 100 + 30, canvas_pos.y + ItemHeight - 2));
            bool inRectCopy = rectCopy.Contains(io.MousePos);
            unsigned int copyColor = inRectCopy ? 0xFF1080FF : 0xFF000000;
            draw_list->AddText(rectCopy.Min, copyColor, "Copy");

            ImRect rectPaste(ImVec2(contentMin.x + 140, canvas_pos.y + 2)
               , ImVec2(contentMin.x + 140 + 30, canvas_pos.y + ItemHeight - 2));
            bool inRectPaste = rectPaste.Contains(io.MousePos);
            unsigned int pasteColor = inRectPaste ? 0xFF1080FF : 0xFF000000;
            draw_list->AddText(rectPaste.Min, pasteColor, "Paste");

            if (inRectCopy && io.MouseReleased[0])
            {
               sequence->Copy();
            }
            if (inRectPaste && io.MouseReleased[0])
            {
               sequence->Paste();
            }
         }
         //

         ImGui::EndChildFrame();
         ImGui::PopStyleColor();
         if (hasScrollBar)
         {
            ImGui::InvisibleButton("scrollBar", scrollBarSize);
            ImVec2 scrollBarMin = ImGui::GetItemRectMin();
            ImVec2 scrollBarMax = ImGui::GetItemRectMax();

            // ratio = number of frames visible in control / number to total frames

            float startFrameOffset = ((float)(firstFrameUsed - sequence->GetFrameMin()) / (float)frameCount) * (canvas_size.x - legendWidth);
            ImVec2 scrollBarA(scrollBarMin.x + legendWidth, scrollBarMin.y - 2);
            ImVec2 scrollBarB(scrollBarMin.x + canvas_size.x, scrollBarMax.y - 1);
            draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);

            ImRect scrollBarRect(scrollBarA, scrollBarB);
            bool inScrollBar = scrollBarRect.Contains(io.MousePos);

            draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, 8);


            ImVec2 scrollBarC(scrollBarMin.x + legendWidth + startFrameOffset, scrollBarMin.y);
            ImVec2 scrollBarD(scrollBarMin.x + legendWidth + barWidthInPixels + startFrameOffset, scrollBarMax.y - 2);
            draw_list->AddRectFilled(scrollBarC, scrollBarD, (inScrollBar || MovingScrollBar) ? 0xFF606060 : 0xFF505050, 6);

            ImRect barHandleLeft(scrollBarC, ImVec2(scrollBarC.x + 14, scrollBarD.y));
            ImRect barHandleRight(ImVec2(scrollBarD.x - 14, scrollBarC.y), scrollBarD);

            bool onLeft = barHandleLeft.Contains(io.MousePos);
            bool onRight = barHandleRight.Contains(io.MousePos);

            static bool sizingRBar = false;
            static bool sizingLBar = false;

            draw_list->AddRectFilled(barHandleLeft.Min, barHandleLeft.Max, (onLeft || sizingLBar) ? 0xFFAAAAAA : 0xFF666666, 6);
            draw_list->AddRectFilled(barHandleRight.Min, barHandleRight.Max, (onRight || sizingRBar) ? 0xFFAAAAAA : 0xFF666666, 6);

            ImRect scrollBarThumb(scrollBarC, scrollBarD);
            static const float MinBarWidth = 44.f;
            if (sizingRBar)
            {
               if (!io.MouseDown[0])
               {
                  sizingRBar = false;
               }
               else
               {
                  float barNewWidth = ImMax(barWidthInPixels + io.MouseDelta.x, MinBarWidth);
                  float barRatio = barNewWidth / barWidthInPixels;
                  framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
                  int newVisibleFrameCount = int((canvas_size.x - legendWidth) / framePixelWidthTarget);
                  int lastFrame = *firstFrame + newVisibleFrameCount;
                  if (lastFrame > sequence->GetFrameMax())
                  {
                     framePixelWidthTarget = framePixelWidth = (canvas_size.x - legendWidth) / float(sequence->GetFrameMax() - *firstFrame);
                  }
               }
            }
            else if (sizingLBar)
            {
               if (!io.MouseDown[0])
               {
                  sizingLBar = false;
               }
               else
               {
                  if (fabsf(io.MouseDelta.x) > FLT_EPSILON)
                  {
                     float barNewWidth = ImMax(barWidthInPixels - io.MouseDelta.x, MinBarWidth);
                     float barRatio = barNewWidth / barWidthInPixels;
                     float previousFramePixelWidthTarget = framePixelWidthTarget;
                     framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
                     float newVisibleFrameCount = visibleFrameCount / barRatio;
                     float newFirstFrame = *firstFrame + newVisibleFrameCount - visibleFrameCount;
                     newFirstFrame = ImClamp(newFirstFrame, (float)sequence->GetFrameMin(), ImMax(sequence->GetFrameMax() - visibleFrameCount, (float)sequence->GetFrameMin()));
                     if (std::abs(newFirstFrame - *firstFrame) < FLT_EPSILON)
                     {
                        framePixelWidth = framePixelWidthTarget = previousFramePixelWidthTarget;
                     }
                     else
                     {
                        *firstFrame = newFirstFrame;
                        if(updatedBeat)
                           *updatedBeat = true;
                     }
                  }
               }
            }
            else
            {
               if (MovingScrollBar)
               {
                  if (!io.MouseDown[0])
                  {
                     MovingScrollBar = false;
                  }
                  else
                  {
                     float framesPerPixelInBar = barWidthInPixels / (float)visibleFrameCount;
                     *firstFrame = ((io.MousePos.x - panningViewSource.x) / framesPerPixelInBar) - panningViewFrame;
                     *firstFrame = ImClamp(*firstFrame, (double)sequence->GetFrameMin(),
                        ImMax((double)(sequence->GetFrameMax() - visibleFrameCount), (double)sequence->GetFrameMin()));
                  
                     if(updatedBeat)
                        *updatedBeat = true;
                  }
               }
               else
               {
                  if (scrollBarThumb.Contains(io.MousePos) && ImGui::IsMouseClicked(0) && firstFrame && movingEntry == -1)
                  {
                     MovingScrollBar = true;
                     panningViewSource = io.MousePos;
                     panningViewFrame = -*firstFrame;
                  }
                  /*if (!sizingRBar && onRight && ImGui::IsMouseClicked(0))
                     sizingRBar = true;
                  if (!sizingLBar && onLeft && ImGui::IsMouseClicked(0))
                     sizingLBar = true;*/

               }
            }
         }
      }

      ImGui::EndGroup();

      if (regionRect.Contains(io.MousePos))
      {
         bool overCustomDraw = false;
         for (auto& custom : customDraws)
         {
            if (custom.customRect.Contains(io.MousePos))
            {
               overCustomDraw = true;
            }
         }
         if (overCustomDraw)
         {
         }
         else
         {
#if 0
            frameOverCursor = *firstFrame + (int)(visibleFrameCount * ((io.MousePos.x - (float)legendWidth - canvas_pos.x) / (canvas_size.x - legendWidth)));
            //frameOverCursor = max(min(*firstFrame - visibleFrameCount / 2, frameCount - visibleFrameCount), 0);

            /**firstFrame -= frameOverCursor;
            *firstFrame *= framePixelWidthTarget / framePixelWidth;
            *firstFrame += frameOverCursor;*/
            if (io.MouseWheel < -FLT_EPSILON)
            {
               *firstFrame -= frameOverCursor;
               *firstFrame = int(*firstFrame * 1.1f);
               framePixelWidthTarget *= 0.9f;
               *firstFrame += frameOverCursor;
            }

            if (io.MouseWheel > FLT_EPSILON)
            {
               *firstFrame -= frameOverCursor;
               *firstFrame = int(*firstFrame * 0.9f);
               framePixelWidthTarget *= 1.1f;
               *firstFrame += frameOverCursor;
            }
#endif
         }
      }

      if (expanded)
      {
         bool overExpanded = SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + 2, canvas_pos.y + 2), !*expanded);
         if (overExpanded && io.MouseReleased[0])
            *expanded = !*expanded;
      }

      if (delEntry != -1)
      {
         sequence->Del(delEntry);
         if (selectedEntry && (*selectedEntry == delEntry || *selectedEntry >= sequence->GetItemCount()))
            *selectedEntry = -1;
      }

      if (dupEntry != -1)
      {
         sequence->Duplicate(dupEntry);
      }
      return ret;
   }
}
