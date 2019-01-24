/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cmath>
#include "phaseseriesrenderer.h"
#include "phaseplot.h"

using namespace Fftchart;

PhaseSeriesRenderer::PhaseSeriesRenderer() : FrequencyBasedSeriesRenderer()
{
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/logx.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/phase.frag");
    m_program.link();
    m_posAttr = m_program.attributeLocation("posAttr");

    m_splineA       = m_program.uniformLocation("splineA");
    m_frequency1    = m_program.uniformLocation("frequency1");
    m_frequency2    = m_program.uniformLocation("frequency2");

    m_widthUniform  = m_program.uniformLocation("width");
    m_colorUniform  = m_program.uniformLocation("m_color");
    m_matrixUniform = m_program.uniformLocation("matrix");
    m_minmaxUniform = m_program.uniformLocation("minmax");
    m_screenUniform = m_program.uniformLocation("screen");
}
void PhaseSeriesRenderer::renderSeries()
{
    if (!m_source->active())
        return;

    PhasePlot *plot = static_cast<PhasePlot*>(m_item->parent());
    GLfloat vertices[8];
    constexpr float
            F_PI = static_cast<float>(M_PI),
            D_PI = 2 * F_PI;

    float value = 0.f, tvalue = 0.f, ltvalue = 0.f, lValue = 0.f;

    setUniforms();
    openGLFunctions->glVertexAttribPointer(static_cast<GLuint>(m_posAttr), 2, GL_FLOAT, GL_FALSE, 0, vertices);
    openGLFunctions->glEnableVertexAttribArray(0);

    float xadd, xmul;
    xadd = -1.0f * logf(plot->xAxis()->min());
    xmul = m_width / logf(plot->xAxis()->max() / plot->xAxis()->min());

    /*
     * Draw quad for each band from -PI to +PI (full height)
     * pass spline data to shaders
     * fragment shader draws phase spline function
     */
    auto accumulate = [&value, &lValue, &tvalue, &ltvalue, m_source = m_source] (unsigned int ai)
    {
        tvalue = m_source->phase(ai);
        //make phase linear (non periodic) function
        if (abs(tvalue - ltvalue) > F_PI) {
            //move to next/prev circle
            tvalue += std::copysign(D_PI, ltvalue);
        }

        tvalue += lValue - ltvalue;   //circles count
        lValue = tvalue;
        value += tvalue;
        ltvalue = m_source->phase(ai);
    };
    auto collected = [m_program = &m_program, openGLFunctions = openGLFunctions, &vertices, &value,
                    m_splineA = m_splineA, m_frequency1 = m_frequency1, m_frequency2 = m_frequency2,
                    xadd, xmul]
            (float f1, float f2, GLfloat *ac)
    {
        vertices[0] =  f1;
        vertices[1] = -D_PI;
        vertices[2] =  f1;
        vertices[3] =  D_PI;
        vertices[4] =  f2;
        vertices[5] =  D_PI;
        vertices[6] =  f2;
        vertices[7] = -D_PI;

        m_program->setUniformValueArray(m_splineA, ac, 1, 4);
        float fx1 = (logf(f1) + xadd) * xmul;
        float fx2 = (logf(f2) + xadd) * xmul;
        m_program->setUniformValue(m_frequency1, fx1);
        m_program->setUniformValue(m_frequency2, fx2);
        openGLFunctions->glDrawArrays(GL_QUADS, 0, 4);

        value = 0.0f;
    };

    iterateForSpline(plot->pointsPerOctave(), &value, accumulate, collected);

    openGLFunctions->glDisableVertexAttribArray(0);
}